package bgu.spl.net.impl.stomp;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

public class serverDataBase {
    public class User{
        private int userId; 
        private String userName;
        private String password;
        private boolean loggedIn;
        private int connectionId;
        private ConcurrentHashMap<String, String> subscriptions;

        public User(int userId, String userName, String password, int connectionId) {
            this.userId = userId;
            this.userName = userName;
            this.password = password;
            this.loggedIn = false;
            this.connectionId = connectionId;
            this.subscriptions = new ConcurrentHashMap<>();
        }

        public int getUserId() {
            return userId;
        }

        public String getUserName() {
            return userName;
        }

        public String getPassword() {
            return password;
        }

        public boolean isLoggedIn() {
            return loggedIn;
        }

        public void setLoggedIn(boolean loggedIn){
            this.loggedIn = loggedIn;
        }

        public ConcurrentHashMap<String, String> getSubscriptions() {
            return subscriptions;
        }

        public void addSubscription(String destination, String id) {
            subscriptions.put(id, destination);
        }

        public void removeSubscription(String id) {
            subscriptions.remove(id);
        }

        public String getDestination(String id) {
            return subscriptions.get(id);
        }

        public boolean isSubscribed(String destination) {
            return subscriptions.containsValue(destination);
        }

        public String getSubscriptionId(String destination) {
            for(String key :subscriptions.keySet() ){
                if(subscriptions.get(key).equals(destination)){
                    return key;
                }
            }
            return "";
            
        }

        public int getConnectionId() {
            return connectionId;
        }

        public void resetSubscriptions() {
            subscriptions.clear();
        }
    }

    //serverDataBase is a singleton class that holds all the data of the server.
    // and is responsible for the data base operations.


    private static serverDataBase instance = null;
    //inv : userName is unique
    private ConcurrentHashMap<String, User> users;
    //inv : subscriptions is unique
    private ConcurrentHashMap<String, List<User>> subscriptions;

    private serverDataBase() {
        users = new ConcurrentHashMap<>();
        subscriptions = new ConcurrentHashMap<>();
    }

    public static serverDataBase getInstance() {
        if(instance == null)
            instance = new serverDataBase();
        return instance;

            
    }

    public ConcurrentHashMap<String, User> getUsers() {
        return users;
    }

    public ConcurrentHashMap<String, List<User>> getSubscriptions() {
        return subscriptions;
    }

    public void addUser(String userName, String password, int connectionId) {
        users.put(userName, new User(users.size(), userName, password, connectionId));
    }

    public boolean isUserExist(String userName) {
        return users.containsKey(userName);
    }

    public boolean isPasswordCorrect(String userName, String password) {
        if(users.get(userName) == null){return false;}
        return users.get(userName).getPassword().equals(password);
    }

    public boolean isUserLoggedIn(String userName) {
        if(users.get(userName)==null) return false;
        return users.get(userName).isLoggedIn();
    }

    public void setLoggedIn(String userName, boolean loggedIn) {
        users.get(userName).setLoggedIn(loggedIn);
    }

    public void addSubscription(String userName, String id, String destination) {
        users.get(userName).addSubscription(destination,id);
        if (subscriptions.containsKey(destination)) {
            subscriptions.get(destination).add(users.get(userName));
            users.get(userName).addSubscription(destination, id);
        } else {
            users.get(userName).addSubscription(destination, id);
            List<User> list = new ArrayList<User>();
            list.add(users.get(userName));
            subscriptions.put(destination, list);
        }
    }

    public void removeSubscription(String userName, String id) {
        if(users.get(userName) == null){return;}
        String destination = users.get(userName).getDestination(id);
        users.get(userName).removeSubscription(id);
        subscriptions.get(destination).remove(users.get(userName));
    }

    public boolean isSubscribed(String userName, String destination) {
        if(users.get(userName) == null){return false;}
        return users.get(userName).isSubscribed(destination);
    }

    public String getSubscriptionId(String userName, String destination) {
        if(users.get(userName) == null){return null;}
        return users.get(userName).getSubscriptionId(destination);
    }

    public String getDestination(String userName, String id) {
        if(users.get(userName) == null){return null;}
        return users.get(userName).getDestination(id);
    }

    public List<User> getSubscribers(String destination) {
        return subscriptions.get(destination);
    }


    public User getUserByConnectionId(int connectionId) {
        for (User user : users.values()) {
            if (user.getConnectionId() == connectionId)
                return user;
        }
        return null;
    }

    public void disconnectUser (String userName){
        User user = users.get(userName);
        for (String key : subscriptions.keySet()) {
            if (user.getSubscriptions().containsKey(subscriptions.get(key)))
                subscriptions.get(key).remove(user);
        }

        users.get(userName).resetSubscriptions();
        users.get(userName).setLoggedIn(false);
    }

    public void setConnectionId(String user, int connectionId) {
        users.get(user).connectionId = connectionId;
    }
}
