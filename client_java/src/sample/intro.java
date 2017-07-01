package sample;

import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ConnectException;
import java.net.Socket;
import java.net.UnknownHostException;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

//
public class intro extends JDialog {

	String ID;
	String IP;
	JTextField ip, id;
	JLabel lb3;
	JDialog a;
	public static Socket socket;
	RW rw;
	static Object KEY = new Object(); // 동기화를 위한 변수
	int con = 0;
	OutputStream outstream;
	InputStream instream;

	public void show() {
		a = new JDialog();
		a.setTitle("Login");
		a.setBounds(100, 100, 500, 300);

		JPanel jp = new JPanel();
		jp.setLayout(null);
		Font f = new Font("Serif", Font.PLAIN, 20);

		JLabel lb1 = new JLabel("IP 주소");
		lb1.setBounds(40, 60, 70, 40);
		lb1.setFont(f);

		ip = new JTextField(100);
		ip.setBounds(130, 60, 200, 40);
		ip.setFont(f);

		JLabel lb2 = new JLabel("User ID");
		lb2.setFont(f);
		lb2.setBounds(40, 130, 70, 40);

		id = new JTextField(100);
		id.setBounds(130, 130, 200, 40);
		id.setFont(f);

		JButton ok = new JButton("OK");
		ok.setFont(f);
		ok.setBounds(350, 80, 80, 70);
		ok.addActionListener(new myActionListner());

		lb3 = new JLabel();
		lb3.setFont(f);
		lb3.setBounds(40, 200, 150, 40);

		jp.add(lb1);
		jp.add(ip);
		jp.add(lb2);
		jp.add(id);
		jp.add(ok);
		jp.add(lb3);
		a.add(jp);

		a.setVisible(true);
	}

	class myActionListner implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			byte[] id_result = new byte[4];
			IP = ip.getText();
			ID = id.getText();

			try {
				if (con == 0) {
					lb3.setText("연결중...");
					lb3.paintImmediately(lb3.getVisibleRect());
					socket = new Socket(IP, 9090);
					rw = new RW();
					outstream = socket.getOutputStream();
					instream = socket.getInputStream();
				}
				rw.my_write(outstream, ID.getBytes(), ID.length());
				if (rw.my_read(instream, id_result) == -1) {
					lb3.setText("중복된 아이디");
					con = 1;
				} else {
					a.dispose();
					a.setVisible(false);
					rw.RW_thread(socket, ID);
				}
			} catch (UnknownHostException e1) {
				// TODO Auto-generated catch block
				//e1.printStackTrace();
				lb3.setText("잘못된 ip");
				return;
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				//e1.printStackTrace();
				lb3.setText("잘못된 ip");
				return;
			}
		}
	}

}
