// Copyright (c) 2022 Proton AG
//
// This file is part of Proton Mail Bridge.
//
// Proton Mail Bridge is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Proton Mail Bridge is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Proton Mail Bridge. If not, see <https://www.gnu.org/licenses/>.


#include "GRPCClient.h"
#include "GRPCUtils.h"
#include "../Exception/Exception.h"
#include "../ProcessMonitor.h"


using namespace google::protobuf;
using namespace grpc;


namespace bridgepp
{


namespace
{


Empty empty; // re-used across client calls.
qint64 const grpcConnectionWaitTimeoutMs = 60000; ///< Timeout for the connection to the already running gRPC server in milliseconds.
qint64 const grpcConnectionRetryDelayMs = 10000; ///< Retry delay for the gRPC connection in milliseconds.


}


//****************************************************************************************************************************************************
//
//****************************************************************************************************************************************************
void GRPCClient::removeServiceConfigFile()
{
    QString const path = grpcServerConfigPath();
    if (!QFile(path).exists())
        return;
    if (!QFile().remove(path))
        throw Exception("Could not remove gRPC service config file.");
}


//****************************************************************************************************************************************************
/// \param[in] timeoutMs The timeout in milliseconds
/// \param[in] serverProcess An optional server process to monitor. If the process it, no need and retry, as connexion cannot be established. Ignored if null.
/// \return The service config.
//****************************************************************************************************************************************************
GRPCConfig GRPCClient::waitAndRetrieveServiceConfig(qint64 timeoutMs, ProcessMonitor *serverProcess)
{
    QString const path = grpcServerConfigPath();
    QFile file(path);

    QElapsedTimer timer;
    timer.start();
    bool found = false;
    while (true)
    {
        if (serverProcess && serverProcess->getStatus().ended)
            throw Exception("Bridge application exited before providing a gRPC service configuration file.");

        if (file.exists())
        {
            found = true;
            break;
        }
        if (timer.elapsed() > timeoutMs)
            break;
        QThread::msleep(100);
    }

    if (!found)
        throw Exception("Server did not provide gRPC service configuration in time.");

    GRPCConfig sc;
    if (!sc.load(path))
        throw Exception("The gRPC service configuration file is invalid.");

    return sc;
}


//****************************************************************************************************************************************************
/// \param[in] log The log
//****************************************************************************************************************************************************
void GRPCClient::setLog(Log *log)
{
    log_ = log;
}


//****************************************************************************************************************************************************
/// \param[out] outError If the function returns false, this variable contains a description of the error.
/// \param[in] serverProcess An optional server process to monitor. If the process it, no need and retry, as connexion cannot be established. Ignored if null.
/// \return true iff the connection was successful.
//****************************************************************************************************************************************************
bool GRPCClient::connectToServer(GRPCConfig const &config, ProcessMonitor *serverProcess, QString &outError)
{
    try
    {
        serverToken_ = config.token.toStdString();
        SslCredentialsOptions opts;
        opts.pem_root_certs += config.cert.toStdString();

        QString const address = QString("127.0.0.1:%1").arg(config.port);
        channel_ = CreateChannel(address.toStdString(), grpc::SslCredentials(opts));
        if (!channel_)
            throw Exception("Channel creation failed.");

        stub_ = Bridge::NewStub(channel_);
        if (!stub_)
            throw Exception("Stub creation failed.");

        QDateTime const giveUpTime = QDateTime::currentDateTime().addMSecs(grpcConnectionWaitTimeoutMs); // if we reach giveUpTime without connecting, we give up
        int i = 0;
        while (true)
        {
            if (serverProcess && serverProcess->getStatus().ended)
                throw Exception("Bridge application ended before gRPC connexion could be established.");

            this->logInfo(QString("Connection to gRPC server at %1. attempt #%2").arg(address).arg(++i));

            if (channel_->WaitForConnected(gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_millis(grpcConnectionRetryDelayMs, GPR_TIMESPAN))))
                break; // connection established.

            if (QDateTime::currentDateTime() > giveUpTime)
                throw Exception("Connection to the RPC server failed.");
        }

        if (channel_->GetState(true) != GRPC_CHANNEL_READY)
            throw Exception("connection check failed.");

        this->logInfo("Successfully connected to gRPC server.");

        QString const clientToken = QUuid::createUuid().toString();
        QString clientConfigPath = createClientConfigFile(clientToken);
        if (clientConfigPath.isEmpty())
            throw Exception("gRPC client config could not be saved.");
        this->logInfo(QString("Client config file was saved to '%1'").arg(QDir::toNativeSeparators(clientConfigPath)));

        QString returnedClientToken;
        grpc::Status status = this->checkTokens(QDir::toNativeSeparators(clientConfigPath), returnedClientToken);
        QFile(clientConfigPath).remove();
        if (clientToken != returnedClientToken)
            throw Exception("gRPC server returned an invalid token");


        if (!status.ok())
            throw Exception(QString::fromStdString(status.error_message()));

        log_->info("gRPC token was validated");

        return true;
    }
    catch (Exception const &e)
    {
        outError = e.qwhat();
        return false;
    }
}


//****************************************************************************************************************************************************
/// \param[in] clientConfigPath The path to the gRPC client config path.-
/// \param[in] serverToken The token obtained from the server config file.
/// \param[out] outReturnedClientToken The client token returned by the server.
/// \return The status code for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::checkTokens(QString const &clientConfigPath, QString &outReturnedClientToken)
{
    google::protobuf::StringValue request;
    request.set_value(clientConfigPath.toStdString());
    google::protobuf::StringValue response;
    Status status = stub_->CheckTokens(this->clientContext().get(), request, &response);
    if (status.ok())
        outReturnedClientToken = QString::fromStdString(response.value());
    return this->logGRPCCallStatus(status,  __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] level The level of the log entry.
/// \param[in] package The package (component) that triggered the entry.
/// \param[in] message The message.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::addLogEntry(Log::Level level, QString const &package, QString const &message)
{
    AddLogEntryRequest request;
    request.set_level(logLevelToGRPC(level));
    request.set_package(package.toStdString());
    request.set_message(message.toStdString());
    return stub_->AddLogEntry(this->clientContext().get(), request, &empty);
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::guiReady()
{
    return this->logGRPCCallStatus(stub_->GuiReady(this->clientContext().get(), empty, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outIsFirst The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::isFirstGUIStart(bool &outIsFirst)
{
    return this->logGRPCCallStatus(this->getBool(&Bridge::Stub::IsFirstGuiStart, outIsFirst), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outIsOn The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::isAutostartOn(bool &outIsOn)
{
    return this->logGRPCCallStatus(this->getBool(&Bridge::Stub::IsAutostartOn, outIsOn), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] on The new value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setIsAutostartOn(bool on)
{
    return this->logGRPCCallStatus(this->setBool(&Bridge::Stub::SetIsAutostartOn, on), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outEnabled The new value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::isBetaEnabled(bool &outEnabled)
{
    return this->logGRPCCallStatus(this->getBool(&Bridge::Stub::IsBetaEnabled, outEnabled), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] enabled The new value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setIsBetaEnabled(bool enabled)
{
    return this->logGRPCCallStatus(this->setBool(&Bridge::Stub::SetIsBetaEnabled, enabled), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outIsVisible The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::isAllMailVisible(bool &outIsVisible)
{
    return this->logGRPCCallStatus(this->getBool(&Bridge::Stub::IsAllMailVisible, outIsVisible), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] isVisible The new value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setIsAllMailVisible(bool isVisible)
{
    return this->logGRPCCallStatus(this->setBool(&Bridge::Stub::SetIsAllMailVisible, isVisible), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outName The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::colorSchemeName(QString &outName)
{
    return this->logGRPCCallStatus(this->getString(&Bridge::Stub::ColorSchemeName, outName), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] name The new value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setColorSchemeName(QString const &name)
{
    return this->logGRPCCallStatus(this->setString(&Bridge::Stub::SetColorSchemeName, name), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outName The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::currentEmailClient(QString &outName)
{
    return this->logGRPCCallStatus(this->getString(&Bridge::Stub::CurrentEmailClient, outName), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] description The description of the bug.
/// \param[in] address The email address.
/// \param[in] emailClient The email client.
/// \param[in] includeLogs Should the report include the logs.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::reportBug(QString const &description, QString const &address, QString const &emailClient, bool includeLogs)
{
    ReportBugRequest request;
    request.set_ostype(QSysInfo::productType().toStdString());
    request.set_osversion(QSysInfo::prettyProductName().toStdString());
    request.set_description(description.toStdString());
    request.set_address(address.toStdString());
    request.set_emailclient(emailClient.toStdString());
    request.set_includelogs(includeLogs);
    return this->logGRPCCallStatus(stub_->ReportBug(this->clientContext().get(), request, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outUseSSL The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::useSSLForSMTP(bool &outUseSSL)
{
    return this->logGRPCCallStatus(this->getBool(&Bridge::Stub::UseSslForSmtp, outUseSSL), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] useSSL The new value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setUseSSLForSMTP(bool useSSL)
{
    return this->logGRPCCallStatus(this->setBool(&Bridge::Stub::SetUseSslForSmtp, useSSL), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outPort The port.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::portIMAP(int &outPort)
{
    return this->logGRPCCallStatus(this->getInt32(&Bridge::Stub::ImapPort, outPort), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outPort The port.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::portSMTP(int &outPort)
{
    return this->logGRPCCallStatus(this->getInt32(&Bridge::Stub::SmtpPort, outPort), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] portIMAP The IMAP port.
/// \param[in] portSMTP The SMTP port.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::changePorts(int portIMAP, int portSMTP)
{
    ChangePortsRequest request;
    request.set_imapport(portIMAP);
    request.set_smtpport(portSMTP);
    return this->logGRPCCallStatus(stub_->ChangePorts(this->clientContext().get(), request, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] outEnabled The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::isDoHEnabled(bool &outEnabled)
{
    return this->logGRPCCallStatus(this->getBool(&Bridge::Stub::IsDoHEnabled, outEnabled), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] enabled The new value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setIsDoHEnabled(bool enabled)
{
    return this->logGRPCCallStatus(this->setBool(&Bridge::Stub::SetIsDoHEnabled, enabled), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::quit()
{
    // quitting will shut down the gRPC service, to we may get an 'Unavailable' response for the call
    return this->logGRPCCallStatus(stub_->Quit(this->clientContext().get(), empty, &empty), __FUNCTION__, { StatusCode::UNAVAILABLE });
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::restart()
{
    // restarting will shut down the gRPC service, to we may get an 'Unavailable' response for the call
    return this->logGRPCCallStatus(stub_->Restart(this->clientContext().get(), empty, &empty), __FUNCTION__, { StatusCode::UNAVAILABLE });
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::triggerReset()
{
    return this->logGRPCCallStatus(stub_->TriggerReset(this->clientContext().get(), empty, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::forceLauncher(QString const &launcher)
{
    return this->logGRPCCallStatus(this->setString(&Bridge::Stub::ForceLauncher, launcher), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setMainExecutable(QString const &exe)
{
    return this->logGRPCCallStatus(this->setString(&Bridge::Stub::SetMainExecutable, exe), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] port The port to check.
/// \param[out] outFree The result of the check.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::isPortFree(qint32 port, bool &outFree)
{
    Int32Value p;
    p.set_value(port);
    BoolValue isFree;
    Status result = stub_->IsPortFree(this->clientContext().get(), p, &isFree);
    if (result.ok())
        outFree = isFree.value();
    return this->logGRPCCallStatus(result, __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out]  outValue The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::showOnStartup(bool &outValue)
{
    return this->logGRPCCallStatus(this->getBool(&Bridge::Stub::ShowOnStartup, outValue), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outValue The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::showSplashScreen(bool &outValue)
{
    return this->logGRPCCallStatus(this->getBool(&Bridge::Stub::ShowSplashScreen, outValue), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outGoos The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::goos(QString &outGoos)
{
    return this->logGRPCCallStatus(this->getString(&Bridge::Stub::GoOs, outGoos), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outPath The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::logsPath(QUrl &outPath)
{
    return this->logGRPCCallStatus(this->getURLForLocalFile(&Bridge::Stub::LogsPath, outPath), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outPath The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::licensePath(QUrl &outPath)
{
    return this->logGRPCCallStatus(this->getURLForLocalFile(&Bridge::Stub::LicensePath, outPath), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outUrl The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::dependencyLicensesLink(QUrl &outUrl)
{
    return this->logGRPCCallStatus(this->getURL(&Bridge::Stub::DependencyLicensesLink, outUrl), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outVersion The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::version(QString &outVersion)
{
    return this->logGRPCCallStatus(this->getString(&Bridge::Stub::Version, outVersion), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outUrl The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::releaseNotesPageLink(QUrl &outUrl)
{
    return this->logGRPCCallStatus(this->getURL(&Bridge::Stub::ReleaseNotesPageLink, outUrl), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outUrl The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::landingPageLink(QUrl &outUrl)
{
    return this->logGRPCCallStatus(this->getURL(&Bridge::Stub::LandingPageLink, outUrl), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outHostname The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::hostname(QString &outHostname)
{
    return this->logGRPCCallStatus(this->getString(&Bridge::Stub::Hostname, outHostname), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outEnabled The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::isCacheOnDiskEnabled(bool &outEnabled)
{
    return this->logGRPCCallStatus(getBool(&Bridge::Stub::IsCacheOnDiskEnabled, outEnabled), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outPath The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::diskCachePath(QUrl &outPath)
{
    return this->logGRPCCallStatus(this->getURLForLocalFile(&Bridge::Stub::DiskCachePath, outPath), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] enabled Should the cache be enabled.
/// \param[in] path The value for the property.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::changeLocalCache(bool enabled, QUrl const &path)
{
    ChangeLocalCacheRequest request;
    request.set_enablediskcache(enabled);
    request.set_diskcachepath(path.path(QUrl::FullyDecoded).toStdString());
    return this->logGRPCCallStatus(stub_->ChangeLocalCache(this->clientContext().get(), request, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] username The username.
/// \param[in] password The password.
/// \return the status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::login(QString const &username, QString const &password)
{
    LoginRequest request;
    request.set_username(username.toStdString());
    request.set_password(password.toStdString());
    return this->logGRPCCallStatus(stub_->Login(this->clientContext().get(), request, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] username The username.
/// \param[in] code The The 2FA code.
/// \return the status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::login2FA(QString const &username, QString const &code)
{
    LoginRequest request;
    request.set_username(username.toStdString());
    request.set_password(code.toStdString());
    return this->logGRPCCallStatus(stub_->Login2FA(this->clientContext().get(), request, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] username The username.
/// \param[in] password The password.
/// \return the status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::login2Passwords(QString const &username, QString const &password)
{
    LoginRequest request;
    request.set_username(username.toStdString());
    request.set_password(password.toStdString());
    return this->logGRPCCallStatus(stub_->Login2Passwords(this->clientContext().get(), request, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] username The username.
/// \return the status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::loginAbort(QString const &username)
{
    LoginAbortRequest request;
    request.set_username(username.toStdString());
    return this->logGRPCCallStatus(stub_->LoginAbort(this->clientContext().get(), request, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::checkUpdate()
{
    return this->logGRPCCallStatus(this->simpleMethod(&Bridge::Stub::CheckUpdate), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::installUpdate()
{
    return this->logGRPCCallStatus(this->simpleMethod(&Bridge::Stub::InstallUpdate), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setIsAutomaticUpdateOn(bool on)
{
    return this->logGRPCCallStatus(this->setBool(&Bridge::Stub::SetIsAutomaticUpdateOn, on), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::isAutomaticUpdateOn(bool &isOn)
{
    return this->logGRPCCallStatus(this->getBool(&Bridge::Stub::IsAutomaticUpdateOn, isOn), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] userID The user ID.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::logoutUser(QString const &userID)
{
    return this->logGRPCCallStatus(methodWithStringParam(&Bridge::Stub::LogoutUser, userID), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] userID The user ID.
/// \return the status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::removeUser(QString const &userID)
{
    return this->logGRPCCallStatus(methodWithStringParam(&Bridge::Stub::RemoveUser, userID), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] userID The user ID.
/// \param[in] address The email address.
/// \return the status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::configureAppleMail(QString const &userID, QString const &address)
{
    ConfigureAppleMailRequest request;
    request.set_userid(userID.toStdString());
    request.set_address(address.toStdString());
    return this->logGRPCCallStatus(stub_->ConfigureUserAppleMail(this->clientContext().get(), request, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] userID The userID.
/// \param[in] active the new status for the mode.
/// \return The status for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setUserSplitMode(QString const &userID, bool active)
{
    UserSplitModeRequest request;
    request.set_userid(userID.toStdString());
    request.set_active(active);

    return this->logGRPCCallStatus(stub_->SetUserSplitMode(this->clientContext().get(), request, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outUsers The user list.
/// \return The status code for the gRPC call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::getUserList(QList<SPUser> &outUsers)
{
    outUsers.clear();

    UserListResponse response;
    Status status = stub_->GetUserList(this->clientContext().get(), empty, &response);
    if (!status.ok())
        return this->logGRPCCallStatus(status, __FUNCTION__);

    for (int i = 0; i < response.users_size(); ++i)
        outUsers.append(this->parseGRPCUser(response.users(i)));

    return this->logGRPCCallStatus(status, __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] userID The userID.
/// \param[out] outUser The user.
/// \return The status code for the operation.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::getUser(QString const &userID, SPUser &outUser)
{
    StringValue s;
    s.set_value(userID.toStdString());
    grpc::User grpcUser;
    Status status = stub_->GetUser(this->clientContext().get(), s, &grpcUser);

    if (status.ok())
        outUser = parseGRPCUser(grpcUser);

    return this->logGRPCCallStatus(grpc::Status(), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outKeychains The list of available keychains.
/// \return The status for the gRPC coll.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::availableKeychains(QStringList &outKeychains)
{
    outKeychains.clear();
    AvailableKeychainsResponse response;
    Status status = stub_->AvailableKeychains(this->clientContext().get(), empty, &response);
    if (!status.ok())
        return this->logGRPCCallStatus(status, __FUNCTION__);

    for (int i = 0; i < response.keychains_size(); ++i)
        outKeychains.append(QString::fromStdString(response.keychains(i)));

    return this->logGRPCCallStatus(status, __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[out] outKeychain The current keychain.
/// \return The status for the gRPC coll.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::currentKeychain(QString &outKeychain)
{
    return this->logGRPCCallStatus(this->getString(&Bridge::Stub::CurrentKeychain, outKeychain), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] keychain The new current keychain.
/// \return The status for the gRPC coll.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setCurrentKeychain(QString const &keychain)
{
    return this->logGRPCCallStatus(this->setString(&Bridge::Stub::SetCurrentKeychain, keychain), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \return true iff the event stream is active.
//****************************************************************************************************************************************************
bool GRPCClient::isEventStreamActive() const
{
    QMutexLocker locker(&eventStreamMutex_);
    return eventStreamContext_.get();
}


//****************************************************************************************************************************************************
/// \return The status for the gRPC coll.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::runEventStreamReader()
{
    {
        QMutexLocker locker(&eventStreamMutex_);
        if (eventStreamContext_)
            return Status(grpc::ALREADY_EXISTS, "event stream is already active.");
        eventStreamContext_ = this->clientContext();
    }

    EventStreamRequest request;
    request.set_clientplatform(QSysInfo::prettyProductName().toStdString());
    std::unique_ptr<grpc::ClientReader<grpc::StreamEvent>> reader(stub_->RunEventStream(eventStreamContext_.get(), request));
    grpc::StreamEvent event;

    while (reader->Read(&event))
    {
        switch (event.event_case())
        {
        case grpc::StreamEvent::kApp:
            this->processAppEvent(event.app());
            break;
        case grpc::StreamEvent::kLogin:
            this->processLoginEvent(event.login());
            break;
        case grpc::StreamEvent::kUpdate:
            this->processUpdateEvent(event.update());
            break;
        case grpc::StreamEvent::kCache:
            this->processCacheEvent(event.cache());
            break;
        case grpc::StreamEvent::kMailSettings:
            this->processMailSettingsEvent(event.mailsettings());
            break;
        case grpc::StreamEvent::kKeychain:
            this->processKeychainEvent(event.keychain());
            break;
        case grpc::StreamEvent::kMail:
            this->processMailEvent(event.mail());
            break;
        case grpc::StreamEvent::kUser:
            this->processUserEvent(event.user());
            break;
        default:
            this->logDebug(QString("Unknown stream event type: %1").arg(event.event_case()));
        }
    }

    Status result = this->logGRPCCallStatus(reader->Finish(), __FUNCTION__);
    QMutexLocker locker(&eventStreamMutex_);
    eventStreamContext_.reset();
    return result;
}


//****************************************************************************************************************************************************
/// \return The status for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::stopEventStreamReader()
{
    if (!this->isEventStreamActive())
        return Status::OK;
    return this->logGRPCCallStatus(stub_->StopEventStream(this->clientContext().get(), empty, &empty), __FUNCTION__);
}


//****************************************************************************************************************************************************
/// \param[in] level The level of the event.
/// \param[in] message The event message.
//****************************************************************************************************************************************************
void GRPCClient::log(Log::Level level, QString const &message)
{
    if (log_)
        log_->addEntry(level, message);
}


//****************************************************************************************************************************************************
/// \param[in] message The event message.
//****************************************************************************************************************************************************
void GRPCClient::logTrace(QString const &message)
{
    this->log(Log::Level::Trace, message);
}


//****************************************************************************************************************************************************
/// \param[in] message The event message.
//****************************************************************************************************************************************************
void GRPCClient::logDebug(QString const &message)
{
    this->log(Log::Level::Debug, message);
}


//****************************************************************************************************************************************************
/// \param[in] message The event message.
//****************************************************************************************************************************************************
void GRPCClient::logError(QString const &message)
{
    this->log(Log::Level::Error, message);
}


//****************************************************************************************************************************************************
/// \param[in] message The event message.
//****************************************************************************************************************************************************
void GRPCClient::logInfo(QString const &message)
{
    this->log(Log::Level::Info, message);
}


//****************************************************************************************************************************************************
/// \param[in] status The status
/// \param[in] callName The call name.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::logGRPCCallStatus(Status const &status, QString const &callName, QList<grpc::StatusCode> allowedErrors)
{
    if (log_)
    {
        if (status.ok() || allowedErrors.contains(status.error_code()))
            log_->debug(QString("%1()").arg(callName));
        else
            log_->error(QString("%1() FAILED").arg(callName));
    }
    return status;
}


//****************************************************************************************************************************************************
/// \param[in] method the gRPC method to call.
/// \return The status for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::simpleMethod(SimpleMethod method)
{
    return ((*stub_).*method)(this->clientContext().get(), empty, &empty);
}


//****************************************************************************************************************************************************
/// \param[in] setter The setter member function.
/// \param[in] value The bool value.
/// \return The status code for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setBool(BoolSetter setter, bool value)
{
    BoolValue v;
    v.set_value(value);
    return ((*stub_).*setter)(this->clientContext().get(), v, &empty);
}


//****************************************************************************************************************************************************
/// \param[in] getter The getter member function.
/// \param[out] outValue The boolean value.
/// \return The status code for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::getBool(BoolGetter getter, bool &outValue)
{
    BoolValue v;
    Status result = ((*stub_).*getter)(this->clientContext().get(), empty, &v);
    if (result.ok())
        outValue = v.value();
    return result;
}


//****************************************************************************************************************************************************
/// \param[in] setter The setter member function.
/// \param[in] value The bool value.
/// \return The status code for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setInt32(Int32Setter setter, int value)
{
    Int32Value i;
    i.set_value(value);
    return ((*stub_).*setter)(this->clientContext().get(), i, &empty);
}


//****************************************************************************************************************************************************
/// \param[in] getter The getter member function.
/// \param[out] outValue The boolean value.
/// \return The status code for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::getInt32(Int32Getter getter, int &outValue)
{
    Int32Value i;
    Status result = ((*stub_).*getter)(this->clientContext().get(), empty, &i);
    if (result.ok())
        outValue = i.value();
    return result;
}


//****************************************************************************************************************************************************
/// \param[in] setter The setter member function.
/// \param[in] value The string value.
/// \return The status code for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::setString(StringSetter setter, QString const &value)
{
    StringValue s;
    s.set_value(value.toStdString());
    return ((*stub_).*setter)(this->clientContext().get(), s, &empty);
}


//****************************************************************************************************************************************************
/// \param[in] getter The getter member function.
/// \param[out] outValue The string value.
/// \return The status code for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::getString(StringGetter getter, QString &outValue)
{
    StringValue v;
    Status result = ((*stub_).*getter)(this->clientContext().get(), empty, &v);
    if (result.ok())
        outValue = QString::fromStdString(v.value());
    return result;
}


//****************************************************************************************************************************************************
/// \param[in] getter The getter member function.
/// \param[out] outValue The URL value.
/// \return The status code for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::getURLForLocalFile(StringGetter getter, QUrl &outValue)
{
    QString str;
    grpc::Status status = this->getString(getter, str);
    if (status.ok())
        outValue = QUrl::fromLocalFile(str);
    return status;
}


//****************************************************************************************************************************************************
/// \param[in] getter The getter member function.
/// \param[out] outValue The URL value.
/// \return The status code for the call.
//****************************************************************************************************************************************************
grpc::Status GRPCClient::getURL(StringGetter getter, QUrl &outValue)
{
    QString str;
    grpc::Status status = this->getString(getter, str);
    if (status.ok())
        outValue = QUrl(str);
    return status;
}


//****************************************************************************************************************************************************
/// \param[in] method The method to call.
/// \param[in] str The string parameter.
/// \return The status code for the call
//****************************************************************************************************************************************************
grpc::Status GRPCClient::methodWithStringParam(StringParamMethod method, QString const &str)
{
    StringValue s;
    s.set_value(str.toStdString());
    return ((*stub_).*method)(this->clientContext().get(), s, &empty);
}


//****************************************************************************************************************************************************
/// \param[in] grpcUser The gRPC user.
/// \return The user.
//****************************************************************************************************************************************************
SPUser GRPCClient::parseGRPCUser(grpc::User const &grpcUser)
{
    SPUser user = userFromGRPC(grpcUser);
    User *u = user.get();

    connect(u, &User::toggleSplitModeForUser, [&](QString const &userID, bool makeItActive) { this->setUserSplitMode(userID, makeItActive); });
    connect(u, &User::logoutUser, [&](QString const &userID) { this->logoutUser(userID); });
    connect(u, &User::removeUser, [&](QString const &userID) { this->removeUser(userID); });
    connect(u, &User::configureAppleMailForUser, [&](QString const &userID, QString const& address) { this->configureAppleMail(userID, address); });

    return user;
}


//****************************************************************************************************************************************************
/// \param[in] event The event.
//****************************************************************************************************************************************************
void GRPCClient::processAppEvent(AppEvent const &event)
{
    switch (event.event_case())
    {
    case AppEvent::kInternetStatus:
        this->logTrace("App event received: InternetStatus.");
        emit internetStatus(event.internetstatus().connected());
        break;
    case AppEvent::kToggleAutostartFinished:
        this->logTrace("App event received: AutostartFinished.");
        emit toggleAutostartFinished();
        break;
    case AppEvent::kResetFinished:
        this->logTrace("App event received: ResetFinished.");
        emit resetFinished();
        break;
    case AppEvent::kReportBugFinished:
        this->logTrace("App event received: ReportBugFinished.");
        emit reportBugFinished();
        break;
    case AppEvent::kReportBugSuccess:
        this->logTrace("App event received: ReportBugSuccess.");
        emit reportBugSuccess();
        break;
    case AppEvent::kReportBugError:
        this->logTrace("App event received: ReportBugError.");
        emit reportBugError();
        break;
    case AppEvent::kShowMainWindow:
        this->logTrace("App event received: ShowMainWindow.");
        emit showMainWindow();
        break;
    default:
        this->logError("Unknown App event received.");
    }
}


//****************************************************************************************************************************************************
/// \param[in] event The event.
//****************************************************************************************************************************************************
void GRPCClient::processLoginEvent(LoginEvent const &event)
{
    switch (event.event_case())
    {
    case LoginEvent::kError:
    {
        this->logTrace("Login event received: Error.");
        LoginErrorEvent const &error = event.error();
        switch (error.type())
        {
        case USERNAME_PASSWORD_ERROR:
            emit loginUsernamePasswordError(QString::fromStdString(error.message()));
            break;
        case FREE_USER:
            emit loginFreeUserError();
            break;
        case CONNECTION_ERROR:
            emit loginConnectionError(QString::fromStdString(error.message()));
            break;
        case TFA_ERROR:
            emit login2FAError(QString::fromStdString(error.message()));
            break;
        case TFA_ABORT:
            emit login2FAErrorAbort(QString::fromStdString(error.message()));
            break;
        case TWO_PASSWORDS_ERROR:
            emit login2PasswordError(QString::fromStdString(error.message()));
            break;
        case TWO_PASSWORDS_ABORT:
            emit login2PasswordErrorAbort(QString::fromStdString(error.message()));
            break;
        default:
            this->logError("Unknown login error event received.");
            break;
        }
        break;
    }
    case LoginEvent::kTfaRequested:
        this->logTrace("Login event received: TfaRequested.");
        emit login2FARequested(QString::fromStdString(event.tfarequested().username()));
        break;
    case LoginEvent::kTwoPasswordRequested:
        this->logTrace("Login event received: TwoPasswordRequested.");
        emit login2PasswordRequested();
        break;
    case LoginEvent::kFinished:
        this->logTrace("Login event received: Finished.");
        emit loginFinished(QString::fromStdString(event.finished().userid()));
        break;
    case LoginEvent::kAlreadyLoggedIn:
        this->logTrace("Login event received: AlreadyLoggedIn.");
        emit loginAlreadyLoggedIn(QString::fromStdString(event.finished().userid()));
        break;
    default:
        this->logError("Unknown Login event received.");
        break;
    }
}


//****************************************************************************************************************************************************
/// \param[in] event The event.
//****************************************************************************************************************************************************
void GRPCClient::processUpdateEvent(UpdateEvent const &event)
{
    switch (event.event_case())
    {
    case UpdateEvent::kError:
    {
        this->logTrace("Update event received: Error.");

        UpdateErrorEvent const &errorEvent = event.error();
        switch (errorEvent.type())
        {
        case UPDATE_MANUAL_ERROR:
            emit updateManualError();
            break;
        case UPDATE_FORCE_ERROR:
            emit updateForceError();
            break;
        case UPDATE_SILENT_ERROR:
            emit updateSilentError();
            break;
        default:
            this->logError("Unknown update error received.");
            break;
        }
        break;
    }
    case UpdateEvent::kManualReady:
        this->logTrace("Update event received: ManualReady.");
        emit updateManualReady(QString::fromStdString(event.manualready().version()));
        break;
    case UpdateEvent::kManualRestartNeeded:
        this->logTrace("Update event received: kManualRestartNeeded.");
        emit updateManualRestartNeeded();
        break;
    case UpdateEvent::kForce:
        this->logTrace("Update event received: kForce.");
        emit updateForce(QString::fromStdString(event.force().version()));
        break;
    case UpdateEvent::kSilentRestartNeeded:
        this->logTrace("Update event received: kSilentRestartNeeded.");
        emit updateSilentRestartNeeded();
        break;
    case UpdateEvent::kIsLatestVersion:
        this->logTrace("Update event received: kIsLatestVersion.");
        emit updateIsLatestVersion();
        break;
    case UpdateEvent::kCheckFinished:
        this->logTrace("Update event received: kCheckFinished.");
        emit checkUpdatesFinished();
        break;
    case UpdateEvent::kVersionChanged:
        this->logTrace("Update event received: kVersionChanged.");
        emit updateVersionChanged();
        break;
    default:
        this->logError("Unknown Update event received.");
        break;
    }
}


//****************************************************************************************************************************************************
/// \param[in] event The event.
//****************************************************************************************************************************************************
void GRPCClient::processCacheEvent(CacheEvent const &event)
{
    switch (event.event_case())
    {
    case CacheEvent::kError:
    {
        switch (event.error().type())
        {
        case CACHE_UNAVAILABLE_ERROR:
            this->logError("Cache error received: cacheUnavailable.");
            emit cacheUnavailable();
            break;
        case CACHE_CANT_MOVE_ERROR:
            this->logError("Cache error received: cacheCantMove.");
            emit cacheCantMove();
            break;
        case DISK_FULL:
            this->logError("Cache error received: diskFull.");
            emit diskFull();
            break;
        default:
            this->logError("Unknown cache error event received.");
            break;
        }
        break;
    }

    case CacheEvent::kLocationChangedSuccess:
        this->logTrace("Cache event received: LocationChangedSuccess.");
        emit cacheLocationChangeSuccess();
        break;

    case CacheEvent::kChangeLocalCacheFinished:
        emit changeLocalCacheFinished(event.changelocalcachefinished().willrestart());
        this->logTrace("Cache event received: ChangeLocalCacheFinished.");
        break;

    case CacheEvent::kIsCacheOnDiskEnabledChanged:
        this->logTrace("Cache event received: IsCacheOnDiskEnabledChanged.");
        emit isCacheOnDiskEnabledChanged(event.iscacheondiskenabledchanged().enabled());
        break;

    case CacheEvent::kDiskCachePathChanged:
        this->logTrace("Cache event received: DiskCachePathChanged.");
        emit diskCachePathChanged(QUrl::fromLocalFile(QString::fromStdString(event.diskcachepathchanged().path())));
        break;

    default:
        this->logError("Unknown Cache event received.");
    }
}


//****************************************************************************************************************************************************
/// \param[in] event The event.
//****************************************************************************************************************************************************
void GRPCClient::processMailSettingsEvent(MailSettingsEvent const &event)
{
    switch (event.event_case())
    {
    case MailSettingsEvent::kError:
        this->logTrace("MailSettings event received: Error.");
        switch (event.error().type())
        {
        case IMAP_PORT_ISSUE:
            emit portIssueIMAP();
            break;
        case SMTP_PORT_ISSUE:
            emit portIssueSMTP();
            break;
        default:
            this->logError("Unknown mail settings error event received.");
            break;
        }

    case MailSettingsEvent::kUseSslForSmtpFinished:
        this->logTrace("MailSettings event received: UseSslForSmtpFinished.");
        emit toggleUseSSLFinished();
        break;
    case MailSettingsEvent::kChangePortsFinished:
        this->logTrace("MailSettings event received: ChangePortsFinished.");
        emit changePortFinished();
        break;
    default:
        this->logError("Unknown MailSettings event received.");
    }
}


//****************************************************************************************************************************************************
/// \param[in] event The event.
//****************************************************************************************************************************************************
void GRPCClient::processKeychainEvent(KeychainEvent const &event)
{
    switch (event.event_case())
    {
    case KeychainEvent::kChangeKeychainFinished:
        this->logTrace("Keychain event received: ChangeKeychainFinished.");
        emit changeKeychainFinished();
        break;
    case KeychainEvent::kHasNoKeychain:
        this->logTrace("Keychain event received: HasNoKeychain.");
        emit hasNoKeychain();
        break;
    case KeychainEvent::kRebuildKeychain:
        this->logTrace("Keychain event received: RebuildKeychain.");
        emit rebuildKeychain();
        break;
    default:
        this->logError("Unknown Keychain event received.");
    }
}


//****************************************************************************************************************************************************
/// \param[in] event The event.
//****************************************************************************************************************************************************
void GRPCClient::processMailEvent(MailEvent const &event)
{
    switch (event.event_case())
    {
    case MailEvent::kNoActiveKeyForRecipientEvent:
    {
        QString const email = QString::fromStdString(event.noactivekeyforrecipientevent().email());
        this->logTrace(QString("Mail event received: NoActiveKeyForRecipient (email = %1).").arg(email));
        emit noActiveKeyForRecipient(email);
        break;
    }
    case MailEvent::kAddressChanged:
        this->logTrace("Mail event received: AddressChanged.");
        emit addressChanged(QString::fromStdString(event.addresschanged().address()));
        break;
    case MailEvent::kAddressChangedLogout:
        this->logTrace("Mail event received: AddressChangedLogout.");
        emit addressChangedLogout(QString::fromStdString(event.addresschangedlogout().address()));
        break;
    case MailEvent::kApiCertIssue:
        emit apiCertIssue();
        this->logTrace("Mail event received: ApiCertIssue.");
        break;
    default:
        this->logError("Unknown Mail event received.");
    }
}


//****************************************************************************************************************************************************
/// \param[in] event The event.
//****************************************************************************************************************************************************
void GRPCClient::processUserEvent(UserEvent const &event)
{
    switch (event.event_case())
    {
    case UserEvent::kToggleSplitModeFinished:
    {
        QString const userID = QString::fromStdString(event.togglesplitmodefinished().userid());
        this->logTrace(QString("User event received: ToggleSplitModeFinished (userID = %1).").arg(userID));
        emit toggleSplitModeFinished(userID);
        break;
    }
    case UserEvent::kUserDisconnected:
    {
        QString const username = QString::fromStdString(event.userdisconnected().username());
        this->logTrace(QString("User event received: UserDisconnected (username =  %1).").arg(username));
        emit userDisconnected(username);
        break;
    }
    case UserEvent::kUserChanged:
    {
        QString const userID = QString::fromStdString(event.userchanged().userid());
        this->logTrace(QString("User event received: UserChanged (userID = %1).").arg(userID));
        emit userChanged(userID);
        break;
    }
    default:
        this->logError("Unknown User event received.");
    }
}


//****************************************************************************************************************************************************
/// \return The context with the server token in the metadata
//****************************************************************************************************************************************************
UPClientContext GRPCClient::clientContext() const
{
    auto ctx = std::make_unique<grpc::ClientContext>();
    ctx->AddMetadata(grpcMetadataServerTokenKey, serverToken_);
    return ctx;
}


} // namespace bridgepp
