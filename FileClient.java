import java.io.File;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.filechooser.FileSystemView;
import java.io.*;
import java.net.Socket;

public class FileClient {

	static private Socket s;
	//private static String filename;
	
	private FileClient(String host, int port, String file) {
		try {
			s = new Socket(host, port);
			sendFile(file);
		} catch (Exception e) {
			e.printStackTrace();
		}		
	}
	
	private void sendFile(String file) throws IOException {
		DataOutputStream dos = new DataOutputStream(s.getOutputStream());
		FileInputStream fis = new FileInputStream(file);
		File f=new File(file);
		byte[] buffer = new byte[6000];
		dos.writeInt((int)f.length());
		while (fis.read(buffer) > 0) {
			dos.write(buffer);
		}
		
		fis.close();
		//dos.close();	
	}
	

	public static void main(String[] args)throws Exception {

		JFileChooser jfc = new JFileChooser(FileSystemView.getFileSystemView().getHomeDirectory());
		
		int returnValue = jfc.showOpenDialog(null);
		// int returnValue = jfc.showSaveDialog(null);
		JFrame frame=new JFrame();
		if (returnValue == JFileChooser.APPROVE_OPTION) {
			File selectedFile = jfc.getSelectedFile();
		//String buffer=null;
		FileClient fc=new FileClient("127.0.0.1",6789,selectedFile.getAbsolutePath());
		DataInputStream dis=new DataInputStream(s.getInputStream());
		String s=dis.readUTF();
		JOptionPane.showMessageDialog(frame,s);
		/*switch(a)
		{
			case 0: System.out.println("Not a Crop");
					JOptionPane.showMessageDialog(frame,"Not a Crop");
			case 1: 
		}*/
	}

}
}