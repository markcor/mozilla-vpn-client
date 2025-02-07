/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include <QList>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>

#include "authenticationlistener.h"
#include "errorhandler.h"

struct MozillaVPNPrivate;
class CaptivePortal;
class CaptivePortalDetection;
class ConnectionBenchmark;
class ConnectionHealth;
class Controller;
class DeviceModel;
class FeedbackCategoryModel;
class IpAddressLookup;
class Keys;
class Location;
class NetworkWatcher;
class ProfileFlow;
class ReleaseMonitor;
class ServerCountryModel;
class ServerData;
class ServerLatency;
class StatusIcon;
class SubscriptionData;
class SupportCategoryModel;
class Telemetry;
class Theme;
class User;

class MozillaVPN final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MozillaVPN)

 public:
  enum State {
    StateAuthenticating,
    StateBackendFailure,
    StateBillingNotAvailable,
    StateDeviceLimit,
    StateInitialize,
    StateMain,
    StatePostAuthentication,
    StateSubscriptionBlocked,
    StateSubscriptionNeeded,
    StateSubscriptionInProgress,
    StateSubscriptionNotValidated,
    StateTelemetryPolicy,
    StateUpdateRequired,
  };
  Q_ENUM(State);

  enum UserState {
    // The user is not authenticated and there is not a logging-out operation
    // in progress. Maybe we are running the authentication flow (to know if we
    // are running the authentication flow, please use the
    // `StateAuthenticating` state).
    UserNotAuthenticated,

    // The user is authenticated and there is not a logging-out operation in
    // progress.
    UserAuthenticated,

    // We are logging out the user. There are a few steps to run in order to
    // complete the logout. In the meantime, the user should be considered as
    // not-authenticated. The next state will be `UserNotAuthenticated`.
    UserLoggingOut,
  };
  Q_ENUM(UserState);

 private:
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(UserState userState READ userState NOTIFY userStateChanged)
  Q_PROPERTY(bool startMinimized READ startMinimized CONSTANT)
  Q_PROPERTY(bool updating READ updating NOTIFY updatingChanged)

 public:
  MozillaVPN();
  ~MozillaVPN();

  static MozillaVPN* instance();

  // This is exactly like the ::instance() method, but it doesn't crash if the
  // MozillaVPN is null. It should be used rarely.
  static MozillaVPN* maybeInstance();

  void initialize();

  State state() const;

  // Exposed QML methods:
  Q_INVOKABLE void authenticate();
  Q_INVOKABLE void cancelAuthentication();
  Q_INVOKABLE void removeDeviceFromPublicKey(const QString& publicKey);
  Q_INVOKABLE void postAuthenticationCompleted();
  Q_INVOKABLE void telemetryPolicyCompleted();
  Q_INVOKABLE void mainWindowLoaded();
  Q_INVOKABLE void activate();
  Q_INVOKABLE void deactivate();
  Q_INVOKABLE void refreshDevices();
  Q_INVOKABLE void update();
  Q_INVOKABLE void backendServiceRestore();
  Q_INVOKABLE void triggerHeartbeat();
  Q_INVOKABLE void submitFeedback(const QString& feedbackText,
                                  const qint8 rating, const QString& category);
  Q_INVOKABLE void createSupportTicket(const QString& email,
                                       const QString& subject,
                                       const QString& issueText,
                                       const QString& category);
  Q_INVOKABLE bool validateUserDNS(const QString& dns) const;
  Q_INVOKABLE void hardResetAndQuit();
  Q_INVOKABLE void exitForUnrecoverableError(const QString& reason);
  Q_INVOKABLE void requestDeleteAccount();
  Q_INVOKABLE void cancelReauthentication();
  Q_INVOKABLE void updateViewShown();

  void authenticateWithType(
      AuthenticationListener::AuthenticationType authenticationType);

  // Private object getters:
  CaptivePortal* captivePortal() const;
  CaptivePortalDetection* captivePortalDetection() const;
  ConnectionBenchmark* connectionBenchmark() const;
  ConnectionHealth* connectionHealth() const;
  Controller* controller() const;
  ServerData* serverData() const;
  DeviceModel* deviceModel() const;
  FeedbackCategoryModel* feedbackCategoryModel() const;
  IpAddressLookup* ipAddressLookup() const;
  SupportCategoryModel* supportCategoryModel() const;
  Keys* keys() const;
  Location* location() const;
  NetworkWatcher* networkWatcher() const;
  ProfileFlow* profileFlow() const;
  ReleaseMonitor* releaseMonitor() const;
  ServerCountryModel* serverCountryModel() const;
  ServerLatency* serverLatency() const;
  StatusIcon* statusIcon() const;
  SubscriptionData* subscriptionData() const;
  Telemetry* telemetry() const;
  Theme* theme() const;
  User* user() const;

  // Called at the end of the authentication flow. We can continue adding the
  // device if it doesn't exist yet, or we can go to OFF state.
  void completeAuthentication(const QByteArray& json, const QString& token);

  void deviceAdded(const QString& deviceName, const QString& publicKey,
                   const QString& privateKey);

  void removeDevice(const QString& publicKey, const QString& source);
  void deviceRemovalCompleted(const QString& publicKey);

  void setJournalPublicAndPrivateKeys(const QString& publicKey,
                                      const QString& privateKey);
  void resetJournalPublicAndPrivateKeys();

  void serversFetched(const QByteArray& serverData);

  void accountChecked(const QByteArray& json);

  void abortAuthentication();

  void silentSwitch();

  void logout();

  UserState userState() const;

  static bool isUserAuthenticated() {
    MozillaVPN* vpn = MozillaVPN::instance();
    return vpn->userState() == MozillaVPN::UserAuthenticated;
  }

  bool startMinimized() const { return m_startMinimized; }

  void setStartMinimized(bool startMinimized) {
    m_startMinimized = startMinimized;
  }

  void setToken(const QString& token);

  [[nodiscard]] bool setServerList(const QByteArray& serverData);

  Q_INVOKABLE void reset(bool forceInitialState);

  bool modelsInitialized() const;

  void quit();

  bool updating() const { return m_updating; }
  void setUpdating(bool updating);

  void heartbeatCompleted(bool success);

  void addCurrentDeviceAndRefreshData(bool refreshProducts);

  void createTicketAnswerRecieved(bool successful) {
    emit ticketCreationAnswer(successful);
  }

  void hardReset();

  static QByteArray authorizationHeader();

  void requestAbout();

 private:
  void setState(State state);

  void maybeStateMain();

  void setUserState(UserState userState);

  void startSchedulingPeriodicOperations();

  void stopSchedulingPeriodicOperations();

  void subscriptionStarted(const QString& productIdentifier);
  void restoreSubscriptionStarted();
  void subscriptionCompleted();
  void subscriptionFailed();
  void subscriptionCanceled();
  void subscriptionFailedInternal(bool canceledByUser);
  void alreadySubscribed();
  void billingNotAvailable();
  void subscriptionNotValidated();

  void completeActivation();

  enum RemovalDeviceOption {
    DeviceNotFound,
    DeviceStillValid,
    DeviceRemoved,
  };

  RemovalDeviceOption maybeRemoveCurrentDevice();

  void controllerStateChanged();

  void maybeRegenerateDeviceKey();

  bool checkCurrentDevice();

  void errorHandled();

  void scheduleRefreshDataTasks(bool refreshProducts);

  static void registerUrlOpenerLabels();

  static void registerErrorHandlers();

 signals:
  void stateChanged();
  void userStateChanged();
  void deviceRemoving(const QString& publicKey);
  void deviceRemoved(const QString& source);
  void aboutNeeded();
  void updatingChanged();
  void accountDeleted();

  void authenticationStarted();
  void authenticationAborted();
  void authenticationCompleted();

  // For Glean
  void initializeGlean();
  void sendGleanPings();
  void setGleanSourceTags(const QStringList& tags);

  void aboutToQuit();

  void ticketCreationAnswer(bool successful);

 private:
  bool m_initialized = false;
  struct MozillaVPNPrivate* m_private = nullptr;

  State m_state = StateInitialize;

  UserState m_userState = UserNotAuthenticated;

  QTimer m_periodicOperationsTimer;
  QTimer m_gleanTimer;

  bool m_startMinimized = false;
  bool m_updating = false;
  bool m_controllerInitialized = false;
};

#endif  // MOZILLAVPN_H
