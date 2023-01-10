package bgu.spl.net.impl.stomp;

import java.util.HashMap;

public class StompFrame{
    public class Frame{
        private String command;
        private HashMap<String, String> headers;
        private HashMap<String, String>[] maps;
        private String body;

        public Frame(String command, HashMap<String, String> headers, HashMap<String, String>[] maps, String body) {
            this.command = command;
            this.headers = headers;
            this.maps = maps;
            this.body = body;
        }

        public String getCommand() {
            return command;
        }

        public HashMap<String, String> getHeaders() {
            return headers;
        }

        public String getBody() {
            return body;
        }

        public HashMap<String, String>[] getMaps() {
            return maps;
        }

        public void setSubscription(String id){
            headers.put(("subscription"),id);
        }

        public void setMessageId(String id){
            headers.put(("message-id"),id);
        }
    }

    class ERRORFrame{
        private String command;
        private HashMap<String, String> headers;
        private Frame errorFrame;
        private String body;

        public ERRORFrame(String command, HashMap<String, String> headers, Frame errorFrame, String error) {
            this.command = command;
            this.headers = headers;
            this.errorFrame = errorFrame;
            this.body = error;
        }

        public String getCommand(){
            return command;
        }
        public HashMap<String, String> getHeaders(){
            return headers;
        }
        public Frame getErrorFrame(){
            return errorFrame;
        }
        public String getBody(){
            return body;
        }


       
    }

    public Frame parse(String frame){
        String[] lines = frame.split("\n");
        String command = lines[0];

        if(!command.equals("SEND")){
            HashMap<String, String> headers = new HashMap<>();
            String body = "";
            for (int i = 1; i < lines.length; i++) {
                if (lines[i].equals("")){
                    for (int j = i + 1; j < lines.length; j++) {
                        body += lines[j];
                    }
                    break;
                }
                String[] header = lines[i].split(":");
                if(header.length>1)
                    headers.put(header[0], header[1]);
            }
            return new Frame(command, headers,null, body);
        }
        else{
            HashMap<String, String> headers = new HashMap<>();
            String[] header = lines[1].split(":");
            headers.put(header[0].trim(), header[1].trim());

            String body = "";
            for (int j = 3; j < lines.length; j++) {
                body += lines[j]+"\n";
            }

            return new Frame(command, headers, null, body);
        }

        
    }

    
    public static String encode(Frame frame){
        String str = frame.getCommand() + "\n";
        HashMap<String, String> headers = frame.getHeaders();
        for(String key : headers.keySet()){
            str += key + ":" + headers.get(key)+"\n";
        }
        if(frame.getBody()!=""){
            str += "\n" + frame.getBody() + "\n" + "\0";
        }
        else{
            str +="\0";
        }
        
        return str;
    }

    public static String encodeError(ERRORFrame frame){
        String str = "ERROR" + "\n";
        HashMap<String, String> headers = frame.getHeaders();
        for(String key : headers.keySet()){
            str += key + ":" + headers.get(key) + "\n";
        }
        str += "\n" + "The massage: " + "\n" + "-----" + "\n";
        String temp = encode(frame.getErrorFrame());
        str += temp.substring(0,temp.length()-1) + "\n" + "-----" + "\n";

        if(frame.getBody()!=""){
            str += "\n" + frame.getBody() + "\n" + "\0";
        }
        else{
            str +="\0";
        }

        return str;
    }

    /**
        * @param receipt-id
        * @param message
        * @param frame
        * @param error
        * @return an ERROR frame
     */
    public ERRORFrame createErrorFrame(String receipt_id, String message, Frame frame, String error){
        HashMap<String, String> headers = new HashMap<>();
        if(receipt_id!=null)
            headers.put("receipt-id", receipt_id);
        headers.put("message", message);
        return new ERRORFrame("ERROR", headers, frame, error);
    }

    /**
        * @param version
        * @return a CONNECTED frame
     */
    public Frame createConnectedFrame(String version){
        HashMap<String, String> headers = new HashMap<>();
        headers.put("version", version);
        return new Frame("CONNECTED", headers, null, "");
    }

    /**
        *@param subscription 
        *@param messageID
        *@param destination
        *@param body
        *@return a MESSAGE frame
    */
    public Frame createMessageFrame(String subscription, String messageID, String destination, String body){
        HashMap<String, String> headers = new HashMap<>();
        headers.put("subscription", subscription);
        headers.put("message-id", messageID);
        headers.put("destination", destination);
        return new Frame("MESSAGE", headers,null,body);
    }

    /**
    * @param receiptID
    * @return a RECEIPT frame
     */
    public Frame createReceiptFrame(String receiptID){
        HashMap<String, String> headers = new HashMap<>();
        headers.put("receipt-id", receiptID);
        return new Frame("RECEIPT", headers,null, "");
    }


}



//StringToFrame DRAFT ---------------
// int index = 9;
// int mapIndex = 0; 
// String[] stops = {"team a updates:", "team b updates:", "description:"};
// for(String stop : stops){
//     if(stop.equals("description:")){
//     headers.put(stop, lines[index+1]);
//     }
//     else{
//     HashMap<String, String> map = new HashMap<>();
//     while(!lines[index].equals(stop)){
//         String[] header = lines[index].split(":");
//         map.put(header[0], header[1]);
//         index++;
//     }
//     maps[mapIndex] = map;
//     mapIndex++;
//     index++;
//     }
// }