/*Server Receiving Image file form android client */

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

public class FileServer extends Thread {
	
	private ServerSocket ss;
	private String filename="Sample.jpg";
	private long filesize;
	
	public FileServer(int port) {
		try {
			ss = new ServerSocket(port);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void run() {
		while (true) {
			try {
				Socket clientSock = ss.accept();
				saveFile(clientSock);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	private void saveFile(Socket clientSock) throws IOException {
		DataInputStream dis = new DataInputStream(clientSock.getInputStream());
		FileOutputStream fos = new FileOutputStream(filename);
		byte[] buffer = new byte[4096];
		int read = 0;
		int totalRead = 0;
		int remaining = dis.readInt(); // For receiving filesize
		while((read = dis.read(buffer, 0, Math.min(buffer.length, remaining))) > 0 ) {
			totalRead += read;
			remaining -= read;
			System.out.println("read " + totalRead + " bytes. Remaining bytes = " + remaining);
			fos.write(buffer, 0, read);
			if(remaining == 0) { 
				System.out.println("File Received");
				System.out.println("Sending Image to Plant_detector");
				try{
					String cmd=new String("hackathon.exe Sample.jpg");
					runProgram(clientSock,cmd.split(" "));
					}catch(Exception e)
					{
						System.out.println(e);
						break;
					}
			}
		}
		
		fos.close();
		dis.close();
	}
	
	public void runProgram(Socket clientSock,String[] program) throws InterruptedException, IOException  
      {  
           Process proc = Runtime.getRuntime().exec (program);  
           InputStream progOutput = proc.getInputStream ();  
           InputStreamReader inputReader=new InputStreamReader(progOutput);  
           BufferedReader reader = new BufferedReader(inputReader);            
           String line;  
           while ((line = reader.readLine()) != null)  
             {  
                     System.out.println(line); 
					DataOutputStream dos=new DataOutputStream(clientSock.getOutputStream());
					dos.writeUTF(line);
					
             }  
           if (0 == proc.waitFor ()) {  
             System.out.println("Process completed successfully"); 
           }  
           else  
           {  
                System.out.println("Their was some issue while running the program"); 
           }  
	
	  }
	public static void main(String[] args) {
		FileServer fs = new FileServer(6789);
		fs.run();
	}

}
