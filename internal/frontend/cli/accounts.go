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

package cli

import (
	"context"
	"strings"

	"github.com/ProtonMail/proton-bridge/v2/internal/bridge"
	"github.com/ProtonMail/proton-bridge/v2/internal/config/settings"
	"github.com/ProtonMail/proton-bridge/v2/internal/users"
	"github.com/abiosoft/ishell"
)

func (f *frontendCLI) listAccounts(c *ishell.Context) {
	spacing := "%-2d: %-20s (%-15s, %-15s)\n"
	f.Printf(bold(strings.ReplaceAll(spacing, "d", "s")), "#", "account", "status", "address mode")
	for idx, userID := range f.bridge.GetUserIDs() {
		user, err := f.bridge.GetUserInfo(userID)
		if err != nil {
			panic(err)
		}
		connected := "disconnected"
		if user.Connected {
			connected = "connected"
		}
		mode := "split"
		if user.Mode == users.CombinedMode {
			mode = "combined"
		}
		f.Printf(spacing, idx, user.Username, connected, mode)
	}
	f.Println()
}

func (f *frontendCLI) showAccountInfo(c *ishell.Context) {
	user := f.askUserByIndexOrName(c)
	if user.ID == "" {
		return
	}

	if !user.Connected {
		f.Printf("Please login to %s to get email client configuration.\n", bold(user.Username))
		return
	}

	if user.Mode == users.CombinedMode {
		f.showAccountAddressInfo(user, user.Addresses[user.Primary])
	} else {
		for _, address := range user.Addresses {
			f.showAccountAddressInfo(user, address)
		}
	}
}

func (f *frontendCLI) showAccountAddressInfo(user users.UserInfo, address string) {
	smtpSecurity := "STARTTLS"
	if f.bridge.GetBool(settings.SMTPSSLKey) {
		smtpSecurity = "SSL"
	}
	f.Println(bold("Configuration for " + address))
	f.Printf("IMAP Settings\nAddress:   %s\nIMAP port: %d\nUsername:  %s\nPassword:  %s\nSecurity:  %s\n",
		bridge.Host,
		f.bridge.GetInt(settings.IMAPPortKey),
		address,
		user.Password,
		"STARTTLS",
	)
	f.Println("")
	f.Printf("SMTP Settings\nAddress:   %s\nSMTP port: %d\nUsername:  %s\nPassword:  %s\nSecurity:  %s\n",
		bridge.Host,
		f.bridge.GetInt(settings.SMTPPortKey),
		address,
		user.Password,
		smtpSecurity,
	)
	f.Println("")
}

func (f *frontendCLI) loginAccount(c *ishell.Context) { //nolint:funlen
	f.ShowPrompt(false)
	defer f.ShowPrompt(true)

	loginName := ""
	if len(c.Args) > 0 {
		user := f.getUserByIndexOrName(c.Args[0])
		if user.ID != "" {
			loginName = user.Addresses[user.Primary]
		}
	}

	if loginName == "" {
		loginName = f.readStringInAttempts("Username", c.ReadLine, isNotEmpty)
		if loginName == "" {
			return
		}
	} else {
		f.Println("Username:", loginName)
	}

	password := f.readStringInAttempts("Password", c.ReadPassword, isNotEmpty)
	if password == "" {
		return
	}

	f.Println("Authenticating ... ")
	client, auth, err := f.bridge.Login(loginName, []byte(password))
	if err != nil {
		f.processAPIError(err)
		return
	}

	if auth.HasTwoFactor() {
		twoFactor := f.readStringInAttempts("Two factor code", c.ReadLine, isNotEmpty)
		if twoFactor == "" {
			return
		}

		err = client.Auth2FA(context.Background(), twoFactor)
		if err != nil {
			f.processAPIError(err)
			return
		}
	}

	mailboxPassword := password
	if auth.HasMailboxPassword() {
		mailboxPassword = f.readStringInAttempts("Mailbox password", c.ReadPassword, isNotEmpty)
	}
	if mailboxPassword == "" {
		return
	}

	f.Println("Adding account ...")
	userID, err := f.bridge.FinishLogin(client, auth, []byte(mailboxPassword))
	if err != nil {
		log.WithField("username", loginName).WithError(err).Error("Login was unsuccessful")
		f.Println("Adding account was unsuccessful:", err)
		return
	}

	user, err := f.bridge.GetUserInfo(userID)
	if err != nil {
		panic(err)
	}

	f.Printf("Account %s was added successfully.\n", bold(user.Username))
}

func (f *frontendCLI) logoutAccount(c *ishell.Context) {
	f.ShowPrompt(false)
	defer f.ShowPrompt(true)

	user := f.askUserByIndexOrName(c)
	if user.ID == "" {
		return
	}
	if f.yesNoQuestion("Are you sure you want to logout account " + bold(user.Username)) {
		if err := f.bridge.LogoutUser(user.ID); err != nil {
			f.printAndLogError("Logging out failed: ", err)
		}
	}
}

func (f *frontendCLI) deleteAccount(c *ishell.Context) {
	f.ShowPrompt(false)
	defer f.ShowPrompt(true)

	user := f.askUserByIndexOrName(c)
	if user.ID == "" {
		return
	}
	if f.yesNoQuestion("Are you sure you want to " + bold("remove account "+user.Username)) {
		clearCache := f.yesNoQuestion("Do you want to remove cache for this account")
		if err := f.bridge.DeleteUser(user.ID, clearCache); err != nil {
			f.printAndLogError("Cannot delete account: ", err)
			return
		}
	}
}

func (f *frontendCLI) deleteAccounts(c *ishell.Context) {
	f.ShowPrompt(false)
	defer f.ShowPrompt(true)

	if !f.yesNoQuestion("Do you really want remove all accounts") {
		return
	}

	for _, userID := range f.bridge.GetUserIDs() {
		user, err := f.bridge.GetUserInfo(userID)
		if err != nil {
			panic(err)
		}
		if err := f.bridge.DeleteUser(user.ID, false); err != nil {
			f.printAndLogError("Cannot delete account ", user.Username, ": ", err)
		}
	}

	c.Println("Keychain cleared")
}

func (f *frontendCLI) deleteEverything(c *ishell.Context) {
	f.ShowPrompt(false)
	defer f.ShowPrompt(true)

	if !f.yesNoQuestion("Do you really want remove everything") {
		return
	}

	f.bridge.FactoryReset()

	c.Println("Everything cleared")

	// Clearing data removes everything (db, preferences, ...) so everything has to be stopped and started again.
	f.restarter.SetToRestart()

	f.Stop()
}

func (f *frontendCLI) changeMode(c *ishell.Context) {
	user := f.askUserByIndexOrName(c)
	if user.ID == "" {
		return
	}

	var targetMode users.AddressMode

	if user.Mode == users.CombinedMode {
		targetMode = users.SplitMode
	} else {
		targetMode = users.CombinedMode
	}

	if !f.yesNoQuestion("Are you sure you want to change the mode for account " + bold(user.Username) + " to " + bold(targetMode)) {
		return
	}

	if err := f.bridge.SetAddressMode(user.ID, targetMode); err != nil {
		f.printAndLogError("Cannot switch address mode:", err)
	}

	f.Printf("Address mode for account %s changed to %s\n", user.Username, targetMode)
}
