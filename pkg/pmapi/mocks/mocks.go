// Code generated by MockGen. DO NOT EDIT.
// Source: github.com/ProtonMail/proton-bridge/pkg/pmapi (interfaces: Client)

// Package mocks is a generated GoMock package.
package mocks

import (
	io "io"
	reflect "reflect"

	crypto "github.com/ProtonMail/gopenpgp/crypto"
	pmapi "github.com/ProtonMail/proton-bridge/pkg/pmapi"
	gomock "github.com/golang/mock/gomock"
)

// MockClient is a mock of Client interface
type MockClient struct {
	ctrl     *gomock.Controller
	recorder *MockClientMockRecorder
}

// MockClientMockRecorder is the mock recorder for MockClient
type MockClientMockRecorder struct {
	mock *MockClient
}

// NewMockClient creates a new mock instance
func NewMockClient(ctrl *gomock.Controller) *MockClient {
	mock := &MockClient{ctrl: ctrl}
	mock.recorder = &MockClientMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockClient) EXPECT() *MockClientMockRecorder {
	return m.recorder
}

// Addresses mocks base method
func (m *MockClient) Addresses() pmapi.AddressList {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Addresses")
	ret0, _ := ret[0].(pmapi.AddressList)
	return ret0
}

// Addresses indicates an expected call of Addresses
func (mr *MockClientMockRecorder) Addresses() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Addresses", reflect.TypeOf((*MockClient)(nil).Addresses))
}

// Auth mocks base method
func (m *MockClient) Auth(arg0, arg1 string, arg2 *pmapi.AuthInfo) (*pmapi.Auth, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Auth", arg0, arg1, arg2)
	ret0, _ := ret[0].(*pmapi.Auth)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Auth indicates an expected call of Auth
func (mr *MockClientMockRecorder) Auth(arg0, arg1, arg2 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Auth", reflect.TypeOf((*MockClient)(nil).Auth), arg0, arg1, arg2)
}

// Auth2FA mocks base method
func (m *MockClient) Auth2FA(arg0 string, arg1 *pmapi.Auth) (*pmapi.Auth2FA, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Auth2FA", arg0, arg1)
	ret0, _ := ret[0].(*pmapi.Auth2FA)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Auth2FA indicates an expected call of Auth2FA
func (mr *MockClientMockRecorder) Auth2FA(arg0, arg1 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Auth2FA", reflect.TypeOf((*MockClient)(nil).Auth2FA), arg0, arg1)
}

// AuthInfo mocks base method
func (m *MockClient) AuthInfo(arg0 string) (*pmapi.AuthInfo, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "AuthInfo", arg0)
	ret0, _ := ret[0].(*pmapi.AuthInfo)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AuthInfo indicates an expected call of AuthInfo
func (mr *MockClientMockRecorder) AuthInfo(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AuthInfo", reflect.TypeOf((*MockClient)(nil).AuthInfo), arg0)
}

// AuthRefresh mocks base method
func (m *MockClient) AuthRefresh(arg0 string) (*pmapi.Auth, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "AuthRefresh", arg0)
	ret0, _ := ret[0].(*pmapi.Auth)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AuthRefresh indicates an expected call of AuthRefresh
func (mr *MockClientMockRecorder) AuthRefresh(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AuthRefresh", reflect.TypeOf((*MockClient)(nil).AuthRefresh), arg0)
}

// ClearData mocks base method
func (m *MockClient) ClearData() {
	m.ctrl.T.Helper()
	m.ctrl.Call(m, "ClearData")
}

// ClearData indicates an expected call of ClearData
func (mr *MockClientMockRecorder) ClearData() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "ClearData", reflect.TypeOf((*MockClient)(nil).ClearData))
}

// CountMessages mocks base method
func (m *MockClient) CountMessages(arg0 string) ([]*pmapi.MessagesCount, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "CountMessages", arg0)
	ret0, _ := ret[0].([]*pmapi.MessagesCount)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// CountMessages indicates an expected call of CountMessages
func (mr *MockClientMockRecorder) CountMessages(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "CountMessages", reflect.TypeOf((*MockClient)(nil).CountMessages), arg0)
}

// CreateAttachment mocks base method
func (m *MockClient) CreateAttachment(arg0 *pmapi.Attachment, arg1, arg2 io.Reader) (*pmapi.Attachment, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "CreateAttachment", arg0, arg1, arg2)
	ret0, _ := ret[0].(*pmapi.Attachment)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// CreateAttachment indicates an expected call of CreateAttachment
func (mr *MockClientMockRecorder) CreateAttachment(arg0, arg1, arg2 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "CreateAttachment", reflect.TypeOf((*MockClient)(nil).CreateAttachment), arg0, arg1, arg2)
}

// CreateDraft mocks base method
func (m *MockClient) CreateDraft(arg0 *pmapi.Message, arg1 string, arg2 int) (*pmapi.Message, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "CreateDraft", arg0, arg1, arg2)
	ret0, _ := ret[0].(*pmapi.Message)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// CreateDraft indicates an expected call of CreateDraft
func (mr *MockClientMockRecorder) CreateDraft(arg0, arg1, arg2 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "CreateDraft", reflect.TypeOf((*MockClient)(nil).CreateDraft), arg0, arg1, arg2)
}

// CreateLabel mocks base method
func (m *MockClient) CreateLabel(arg0 *pmapi.Label) (*pmapi.Label, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "CreateLabel", arg0)
	ret0, _ := ret[0].(*pmapi.Label)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// CreateLabel indicates an expected call of CreateLabel
func (mr *MockClientMockRecorder) CreateLabel(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "CreateLabel", reflect.TypeOf((*MockClient)(nil).CreateLabel), arg0)
}

// CurrentUser mocks base method
func (m *MockClient) CurrentUser() (*pmapi.User, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "CurrentUser")
	ret0, _ := ret[0].(*pmapi.User)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// CurrentUser indicates an expected call of CurrentUser
func (mr *MockClientMockRecorder) CurrentUser() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "CurrentUser", reflect.TypeOf((*MockClient)(nil).CurrentUser))
}

// DecryptAndVerifyCards mocks base method
func (m *MockClient) DecryptAndVerifyCards(arg0 []pmapi.Card) ([]pmapi.Card, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "DecryptAndVerifyCards", arg0)
	ret0, _ := ret[0].([]pmapi.Card)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// DecryptAndVerifyCards indicates an expected call of DecryptAndVerifyCards
func (mr *MockClientMockRecorder) DecryptAndVerifyCards(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "DecryptAndVerifyCards", reflect.TypeOf((*MockClient)(nil).DecryptAndVerifyCards), arg0)
}

// DeleteAttachment mocks base method
func (m *MockClient) DeleteAttachment(arg0 string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "DeleteAttachment", arg0)
	ret0, _ := ret[0].(error)
	return ret0
}

// DeleteAttachment indicates an expected call of DeleteAttachment
func (mr *MockClientMockRecorder) DeleteAttachment(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "DeleteAttachment", reflect.TypeOf((*MockClient)(nil).DeleteAttachment), arg0)
}

// DeleteAuth mocks base method
func (m *MockClient) DeleteAuth() error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "DeleteAuth")
	ret0, _ := ret[0].(error)
	return ret0
}

// DeleteAuth indicates an expected call of DeleteAuth
func (mr *MockClientMockRecorder) DeleteAuth() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "DeleteAuth", reflect.TypeOf((*MockClient)(nil).DeleteAuth))
}

// DeleteLabel mocks base method
func (m *MockClient) DeleteLabel(arg0 string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "DeleteLabel", arg0)
	ret0, _ := ret[0].(error)
	return ret0
}

// DeleteLabel indicates an expected call of DeleteLabel
func (mr *MockClientMockRecorder) DeleteLabel(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "DeleteLabel", reflect.TypeOf((*MockClient)(nil).DeleteLabel), arg0)
}

// DeleteMessages mocks base method
func (m *MockClient) DeleteMessages(arg0 []string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "DeleteMessages", arg0)
	ret0, _ := ret[0].(error)
	return ret0
}

// DeleteMessages indicates an expected call of DeleteMessages
func (mr *MockClientMockRecorder) DeleteMessages(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "DeleteMessages", reflect.TypeOf((*MockClient)(nil).DeleteMessages), arg0)
}

// EmptyFolder mocks base method
func (m *MockClient) EmptyFolder(arg0, arg1 string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "EmptyFolder", arg0, arg1)
	ret0, _ := ret[0].(error)
	return ret0
}

// EmptyFolder indicates an expected call of EmptyFolder
func (mr *MockClientMockRecorder) EmptyFolder(arg0, arg1 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "EmptyFolder", reflect.TypeOf((*MockClient)(nil).EmptyFolder), arg0, arg1)
}

// GetAddresses mocks base method
func (m *MockClient) GetAddresses() (pmapi.AddressList, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "GetAddresses")
	ret0, _ := ret[0].(pmapi.AddressList)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetAddresses indicates an expected call of GetAddresses
func (mr *MockClientMockRecorder) GetAddresses() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetAddresses", reflect.TypeOf((*MockClient)(nil).GetAddresses))
}

// GetAttachment mocks base method
func (m *MockClient) GetAttachment(arg0 string) (io.ReadCloser, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "GetAttachment", arg0)
	ret0, _ := ret[0].(io.ReadCloser)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetAttachment indicates an expected call of GetAttachment
func (mr *MockClientMockRecorder) GetAttachment(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetAttachment", reflect.TypeOf((*MockClient)(nil).GetAttachment), arg0)
}

// GetContactByID mocks base method
func (m *MockClient) GetContactByID(arg0 string) (pmapi.Contact, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "GetContactByID", arg0)
	ret0, _ := ret[0].(pmapi.Contact)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetContactByID indicates an expected call of GetContactByID
func (mr *MockClientMockRecorder) GetContactByID(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetContactByID", reflect.TypeOf((*MockClient)(nil).GetContactByID), arg0)
}

// GetContactEmailByEmail mocks base method
func (m *MockClient) GetContactEmailByEmail(arg0 string, arg1, arg2 int) ([]pmapi.ContactEmail, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "GetContactEmailByEmail", arg0, arg1, arg2)
	ret0, _ := ret[0].([]pmapi.ContactEmail)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetContactEmailByEmail indicates an expected call of GetContactEmailByEmail
func (mr *MockClientMockRecorder) GetContactEmailByEmail(arg0, arg1, arg2 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetContactEmailByEmail", reflect.TypeOf((*MockClient)(nil).GetContactEmailByEmail), arg0, arg1, arg2)
}

// GetEvent mocks base method
func (m *MockClient) GetEvent(arg0 string) (*pmapi.Event, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "GetEvent", arg0)
	ret0, _ := ret[0].(*pmapi.Event)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetEvent indicates an expected call of GetEvent
func (mr *MockClientMockRecorder) GetEvent(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetEvent", reflect.TypeOf((*MockClient)(nil).GetEvent), arg0)
}

// GetMailSettings mocks base method
func (m *MockClient) GetMailSettings() (pmapi.MailSettings, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "GetMailSettings")
	ret0, _ := ret[0].(pmapi.MailSettings)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetMailSettings indicates an expected call of GetMailSettings
func (mr *MockClientMockRecorder) GetMailSettings() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetMailSettings", reflect.TypeOf((*MockClient)(nil).GetMailSettings))
}

// GetMessage mocks base method
func (m *MockClient) GetMessage(arg0 string) (*pmapi.Message, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "GetMessage", arg0)
	ret0, _ := ret[0].(*pmapi.Message)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetMessage indicates an expected call of GetMessage
func (mr *MockClientMockRecorder) GetMessage(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetMessage", reflect.TypeOf((*MockClient)(nil).GetMessage), arg0)
}

// GetPublicKeysForEmail mocks base method
func (m *MockClient) GetPublicKeysForEmail(arg0 string) ([]pmapi.PublicKey, bool, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "GetPublicKeysForEmail", arg0)
	ret0, _ := ret[0].([]pmapi.PublicKey)
	ret1, _ := ret[1].(bool)
	ret2, _ := ret[2].(error)
	return ret0, ret1, ret2
}

// GetPublicKeysForEmail indicates an expected call of GetPublicKeysForEmail
func (mr *MockClientMockRecorder) GetPublicKeysForEmail(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetPublicKeysForEmail", reflect.TypeOf((*MockClient)(nil).GetPublicKeysForEmail), arg0)
}

// Import mocks base method
func (m *MockClient) Import(arg0 []*pmapi.ImportMsgReq) ([]*pmapi.ImportMsgRes, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Import", arg0)
	ret0, _ := ret[0].([]*pmapi.ImportMsgRes)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Import indicates an expected call of Import
func (mr *MockClientMockRecorder) Import(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Import", reflect.TypeOf((*MockClient)(nil).Import), arg0)
}

// IsConnected mocks base method
func (m *MockClient) IsConnected() bool {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "IsConnected")
	ret0, _ := ret[0].(bool)
	return ret0
}

// IsConnected indicates an expected call of IsConnected
func (mr *MockClientMockRecorder) IsConnected() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "IsConnected", reflect.TypeOf((*MockClient)(nil).IsConnected))
}

// KeyRingForAddressID mocks base method
func (m *MockClient) KeyRingForAddressID(arg0 string) *crypto.KeyRing {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "KeyRingForAddressID", arg0)
	ret0, _ := ret[0].(*crypto.KeyRing)
	return ret0
}

// KeyRingForAddressID indicates an expected call of KeyRingForAddressID
func (mr *MockClientMockRecorder) KeyRingForAddressID(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "KeyRingForAddressID", reflect.TypeOf((*MockClient)(nil).KeyRingForAddressID), arg0)
}

// LabelMessages mocks base method
func (m *MockClient) LabelMessages(arg0 []string, arg1 string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "LabelMessages", arg0, arg1)
	ret0, _ := ret[0].(error)
	return ret0
}

// LabelMessages indicates an expected call of LabelMessages
func (mr *MockClientMockRecorder) LabelMessages(arg0, arg1 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "LabelMessages", reflect.TypeOf((*MockClient)(nil).LabelMessages), arg0, arg1)
}

// ListLabels mocks base method
func (m *MockClient) ListLabels() ([]*pmapi.Label, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "ListLabels")
	ret0, _ := ret[0].([]*pmapi.Label)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// ListLabels indicates an expected call of ListLabels
func (mr *MockClientMockRecorder) ListLabels() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "ListLabels", reflect.TypeOf((*MockClient)(nil).ListLabels))
}

// ListMessages mocks base method
func (m *MockClient) ListMessages(arg0 *pmapi.MessagesFilter) ([]*pmapi.Message, int, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "ListMessages", arg0)
	ret0, _ := ret[0].([]*pmapi.Message)
	ret1, _ := ret[1].(int)
	ret2, _ := ret[2].(error)
	return ret0, ret1, ret2
}

// ListMessages indicates an expected call of ListMessages
func (mr *MockClientMockRecorder) ListMessages(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "ListMessages", reflect.TypeOf((*MockClient)(nil).ListMessages), arg0)
}

// Logout mocks base method
func (m *MockClient) Logout() {
	m.ctrl.T.Helper()
	m.ctrl.Call(m, "Logout")
}

// Logout indicates an expected call of Logout
func (mr *MockClientMockRecorder) Logout() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Logout", reflect.TypeOf((*MockClient)(nil).Logout))
}

// MarkMessagesRead mocks base method
func (m *MockClient) MarkMessagesRead(arg0 []string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "MarkMessagesRead", arg0)
	ret0, _ := ret[0].(error)
	return ret0
}

// MarkMessagesRead indicates an expected call of MarkMessagesRead
func (mr *MockClientMockRecorder) MarkMessagesRead(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MarkMessagesRead", reflect.TypeOf((*MockClient)(nil).MarkMessagesRead), arg0)
}

// MarkMessagesUnread mocks base method
func (m *MockClient) MarkMessagesUnread(arg0 []string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "MarkMessagesUnread", arg0)
	ret0, _ := ret[0].(error)
	return ret0
}

// MarkMessagesUnread indicates an expected call of MarkMessagesUnread
func (mr *MockClientMockRecorder) MarkMessagesUnread(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MarkMessagesUnread", reflect.TypeOf((*MockClient)(nil).MarkMessagesUnread), arg0)
}

// ReportBugWithEmailClient mocks base method
func (m *MockClient) ReportBugWithEmailClient(arg0, arg1, arg2, arg3, arg4, arg5, arg6 string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "ReportBugWithEmailClient", arg0, arg1, arg2, arg3, arg4, arg5, arg6)
	ret0, _ := ret[0].(error)
	return ret0
}

// ReportBugWithEmailClient indicates an expected call of ReportBugWithEmailClient
func (mr *MockClientMockRecorder) ReportBugWithEmailClient(arg0, arg1, arg2, arg3, arg4, arg5, arg6 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "ReportBugWithEmailClient", reflect.TypeOf((*MockClient)(nil).ReportBugWithEmailClient), arg0, arg1, arg2, arg3, arg4, arg5, arg6)
}

// SendMessage mocks base method
func (m *MockClient) SendMessage(arg0 string, arg1 *pmapi.SendMessageReq) (*pmapi.Message, *pmapi.Message, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "SendMessage", arg0, arg1)
	ret0, _ := ret[0].(*pmapi.Message)
	ret1, _ := ret[1].(*pmapi.Message)
	ret2, _ := ret[2].(error)
	return ret0, ret1, ret2
}

// SendMessage indicates an expected call of SendMessage
func (mr *MockClientMockRecorder) SendMessage(arg0, arg1 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "SendMessage", reflect.TypeOf((*MockClient)(nil).SendMessage), arg0, arg1)
}

// SendSimpleMetric mocks base method
func (m *MockClient) SendSimpleMetric(arg0, arg1, arg2 string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "SendSimpleMetric", arg0, arg1, arg2)
	ret0, _ := ret[0].(error)
	return ret0
}

// SendSimpleMetric indicates an expected call of SendSimpleMetric
func (mr *MockClientMockRecorder) SendSimpleMetric(arg0, arg1, arg2 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "SendSimpleMetric", reflect.TypeOf((*MockClient)(nil).SendSimpleMetric), arg0, arg1, arg2)
}

// UnlabelMessages mocks base method
func (m *MockClient) UnlabelMessages(arg0 []string, arg1 string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "UnlabelMessages", arg0, arg1)
	ret0, _ := ret[0].(error)
	return ret0
}

// UnlabelMessages indicates an expected call of UnlabelMessages
func (mr *MockClientMockRecorder) UnlabelMessages(arg0, arg1 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "UnlabelMessages", reflect.TypeOf((*MockClient)(nil).UnlabelMessages), arg0, arg1)
}

// Unlock mocks base method
func (m *MockClient) Unlock(arg0 string) (*crypto.KeyRing, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Unlock", arg0)
	ret0, _ := ret[0].(*crypto.KeyRing)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Unlock indicates an expected call of Unlock
func (mr *MockClientMockRecorder) Unlock(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Unlock", reflect.TypeOf((*MockClient)(nil).Unlock), arg0)
}

// UnlockAddresses mocks base method
func (m *MockClient) UnlockAddresses(arg0 []byte) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "UnlockAddresses", arg0)
	ret0, _ := ret[0].(error)
	return ret0
}

// UnlockAddresses indicates an expected call of UnlockAddresses
func (mr *MockClientMockRecorder) UnlockAddresses(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "UnlockAddresses", reflect.TypeOf((*MockClient)(nil).UnlockAddresses), arg0)
}

// UpdateLabel mocks base method
func (m *MockClient) UpdateLabel(arg0 *pmapi.Label) (*pmapi.Label, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "UpdateLabel", arg0)
	ret0, _ := ret[0].(*pmapi.Label)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// UpdateLabel indicates an expected call of UpdateLabel
func (mr *MockClientMockRecorder) UpdateLabel(arg0 interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "UpdateLabel", reflect.TypeOf((*MockClient)(nil).UpdateLabel), arg0)
}

// UpdateUser mocks base method
func (m *MockClient) UpdateUser() (*pmapi.User, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "UpdateUser")
	ret0, _ := ret[0].(*pmapi.User)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// UpdateUser indicates an expected call of UpdateUser
func (mr *MockClientMockRecorder) UpdateUser() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "UpdateUser", reflect.TypeOf((*MockClient)(nil).UpdateUser))
}
