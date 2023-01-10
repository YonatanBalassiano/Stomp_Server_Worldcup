package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.stomp.StompFrame.ERRORFrame;
import bgu.spl.net.impl.stomp.StompFrame.Frame;
import bgu.spl.net.impl.stomp.serverDataBase.User;
import bgu.spl.net.srv.Connections;

class StompMessagingProtocolImpl<T> implements StompMessagingProtocol<T> {
    private boolean shouldTerminate = false;
    serverDataBase dataBase = serverDataBase.getInstance();
    Connections connections;
    int connectionId;

    /**
     * Used to initiate the current client protocol with it's personal connection ID
     * and the connections implementation
     **/
    @Override
    public void start(int connectionId, Connections<T> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    /**
     * process a message received from the client
     * 
     * @param message
     */
    @Override
    public void process(T message) {

        if (message instanceof String) {
            String msg = (String) message;
            System.out.println("message recived is" + msg);
            StompFrame stomp = new StompFrame();
            Frame frame = stomp.parse(msg);

            String command = frame.getCommand();
            User user = dataBase.getUserByConnectionId(connectionId);
            switch (command) {
                case "CONNECT":
                    // case the user is already logged in
                    if (dataBase.isUserLoggedIn(frame.getHeaders().get("login"))) {
                        ERRORFrame errorFrame = stomp.createErrorFrame(frame.getHeaders().get("receipt"),
                                "user logged in", frame, "user already logged in");
                        connections.send(connectionId, StompFrame.encodeError(errorFrame));
                        disconnectUserFromServer(frame.getHeaders().get("login"));
                    } // case the user is NOT logged in
                    else {
                        // case the user is already exist
                        if (dataBase.isUserExist(frame.getHeaders().get("login"))) {
                            if (dataBase.isPasswordCorrect(frame.getHeaders().get("login"),
                                    frame.getHeaders().get("passcode"))) {
                                // need to connect the user
                                dataBase.setConnectionId(frame.getHeaders().get("login"), connectionId);
                                dataBase.setLoggedIn(frame.getHeaders().get("login"), true);

                                Frame connectedFrame = stomp
                                        .createConnectedFrame(frame.getHeaders().get("accept-version"));

                                connections.send(connectionId, StompFrame.encode(connectedFrame));
                            } else {
                                ERRORFrame errorFrame = stomp.createErrorFrame(frame.getHeaders().get("receipt"),
                                        "wrong password", frame, "wrong password");
                                connections.send(connectionId, StompFrame.encodeError(errorFrame));

                            }
                        }
                        // case the user is NOT exist
                        else {
                            dataBase.addUser(frame.getHeaders().get("login"), frame.getHeaders().get("passcode"),
                                    connectionId);
                            dataBase.setLoggedIn(frame.getHeaders().get("login"), true);

                            Frame connectedFrame = stomp.createConnectedFrame(frame.getHeaders().get("accept-version"));

                            System.out.println("connectedFrame: " + StompFrame.encode(connectedFrame));
                            connections.send(connectionId, StompFrame.encode(connectedFrame));
                        }
                    }
                    break;
                case "SUBSCRIBE":
                    if (!dataBase.isSubscribed(user.getUserName(), frame.getHeaders().get("destination"))) {
                        if (frame.getHeaders().get("destination") != null
                                && frame.getHeaders().get("destination") != null) {
                            dataBase.addSubscription(user.getUserName(), frame.getHeaders().get("id"),
                                    frame.getHeaders().get("destination"));
                            Frame receiptFrame = stomp.createReceiptFrame(frame.getHeaders().get("receipt"));
                            connections.send(connectionId, StompFrame.encode(receiptFrame));
                        } else { // error
                            ERRORFrame errorFrame = stomp.createErrorFrame(frame.getHeaders().get("receipt"),
                                    "something missing", frame, "something went wrong, please try again");
                            connections.send(connectionId, StompFrame.encodeError(errorFrame));
                            disconnectUserFromServer(user.getUserName());
                        }
                    }
                    break;
                case "UNSUBSCRIBE":
                    String id = frame.getHeaders().get("id");
                    String destination = id == null ? null : user.getDestination(frame.getHeaders().get("id"));
                    if (destination != null && dataBase.isSubscribed(user.getUserName(), destination)) {
                        dataBase.removeSubscription(user.getUserName(), frame.getHeaders().get("id"));

                        Frame receiptFrame = stomp.createReceiptFrame(frame.getHeaders().get("receipt"));

                        connections.send(connectionId, StompFrame.encode(receiptFrame));
                    } else {
                        ERRORFrame errorFrame = stomp.createErrorFrame(frame.getHeaders().get("receipt"),
                                "not subscribe", frame, "you are not subscribe to this topic");
                        connections.send(connectionId, StompFrame.encodeError(errorFrame));
                        disconnectUserFromServer(user.getUserName());
                    }
                    break;
                case "SEND":
                    if (frame.getHeaders().get("destination") != null && frame.getBody() != null && user.isSubscribed(frame.getHeaders().get("destination"))) {
                        Frame messageFrame = stomp.createMessageFrame("0", "0", frame.getHeaders().get("destination"),
                                frame.getBody());
                        // connections.send(desti, StompFrame.encode(messageFrame));
                        connections.send(frame.getHeaders().get("destination"), messageFrame);
                    } else {
                        ERRORFrame errorFrame = stomp.createErrorFrame(frame.getHeaders().get("receipt"),
                                "something missing", frame, "something went wrong OR you are not Subscribed, please try again");
                        connections.send(connectionId, StompFrame.encodeError(errorFrame));
                        disconnectUserFromServer(user.getUserName());
                    }
                    break;
                case "DISCONNECT":
                    if (dataBase.isUserLoggedIn(user.getUserName())) {
                        Frame receiptFrame = stomp.createReceiptFrame(frame.getHeaders().get("receipt"));
                        connections.send(connectionId, StompFrame.encode(receiptFrame));
                        disconnectUserFromServer(user.getUserName());
                    }
                    break;
            }
        }
    }

    /**
     * @return true if the connection should be terminated
     */
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    public void disconnectUserFromServer(String userName) {

        dataBase.disconnectUser(userName);
        shouldTerminate = true;
        connections.disconnect(connectionId);
    }

}