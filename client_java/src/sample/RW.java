package sample;

import java.awt.Color;
import java.awt.Font;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.lang.Character.UnicodeBlock;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.StringTokenizer;

import javax.swing.JFrame;

public class RW extends JFrame {
	sample frame = new sample();
	sec_thread timer;
	start_thread timer2;
	Socket socket;
	String id;
	int check;
	String id_list[] = new String[4];
	static Object KYKY = new Object();
	int first = 0;
	String beforePoint = "";
	int score[] = { 0, 0, 0, 0 };

	RW() {
		for (int i = 0; i < 4; i++)
			id_list[i] = "";
	}

	public void RW_thread(Socket socket, String id) {
		frame.show();
		check = frame.check;
		this.socket = socket;
		this.id = id;
		idupdate(socket);

		// �쎄린
		new Thread() {
			public void run() {
				try {
					InputStream instream = socket.getInputStream();

					while (true) {
						byte[] message = new byte[200];
						int value;

						value = my_read(instream, message);
						if (value == 0) {
							instream.close();
							interrupt();
						}

						System.out.println("read_thread message : "
								+ new String(message));
						System.out.println("read_thread value : " + value);

						// 채팅
						if (message[0] == '[') {
							frame.setchat(new String(message, "euc-kr"), value);
						}

						// 문제
						else if (message[0] == '<') {
							StringTokenizer tokenizer = new StringTokenizer(
									new String(message).substring(1, value)
											.replaceAll(">", " "));
							System.out.println(new String(message).substring(1,
									value).replaceAll(">", " "));
							String id_m = tokenizer.nextToken().trim();
							System.out.println("id_m" + id_m);
							String keyword_m = tokenizer.nextToken().trim();
							System.out.println("keyword_m : " + keyword_m);

							if (keyword_m.equals("correct!")) {
								int tmp = 0;
								frame.setchat("★★ " + id_m + "님, 정답! ★★",
										3 + id_m.length() + 9);
								for (int i = 0; i < 4; i++) {
									if (id_m.equals(id_list[i]))
										tmp = i;
									frame.table.setValueAt("", i, 2);
								}
								score[tmp] += 10;

								frame.table.setValueAt(score[tmp], tmp, 3);
								frame.jp1.repaint();
								timer.interrupt();

							} else {
								timer = new sec_thread();
								timer.start();
								int tmp = 0;
								if (id_m.equals(id)) {
									for (int i = 0; i < 4; i++) {
										if (id_m.equals(id_list[i])) {
											tmp = i;
											frame.table.setValueAt("◎", i, 2);
											frame.jp1.repaint();
										}
									}
									frame.table.setValueAt(score[tmp], tmp, 3);
									frame.keyword.setText(keyword_m);
								} else {
									for (int i = 0; i < 4; i++) {
										if (id_m.equals(id_list[i])) {
											frame.table.setValueAt("◎", i, 2);
											frame.jp1.repaint();
										}
									}
									frame.keyword.setText("");
									frame.jp3.repaint();
								}

							}
						}

						// 입장
						else if (message[0] == '\'') {
							if (first == 1) {
								synchronized (KYKY) {
									System.out.println("bbb : "
											+ new String(message).substring(1,
													value - 1));
									String id = new String(message).substring(
											1, value - 1);

									frame.setchat("★★ " + id + " is enter. ★★",
											value + 14);
									for (int i = 1; i < 4; i++) {
										if (id_list[i].equals("")) {
											id_list[i] = id;
											System.out.println("i : " + i);
											frame.table.setValueAt(id_list[i],
													i, 1);
											frame.jp1.repaint();
											// if (i == 3) {
											// timer2.start();
											// }
											break;
										}
									}
								}
							}
						}

						// 4명이 다 찼다는 신호
						else if (message[0] == '*') {
							System.out.println("!!!!!!!!full");
							timer2 = new start_thread();
							timer2.start();
						}

						// 게임의 끝
						else if (message[0] == '~') {
							frame.a.setDefaultCloseOperation(EXIT_ON_CLOSE);
							result result = new result(frame.table);
							result.setVisible(true);
						}

						// 그림좌표
						else if (value < 0) {
							getpoint(new String(message));
						}

						// 서버
						else {
							// 색깔넣어주기
							System.out.println(new String(message, "euc-kr")
									.toString());
						}
					}
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
					interrupt();
				}
			}
		}.start();

		// �곌린
		new Thread() {
			public void run() {
				OutputStream outstream;
				try {
					String message;
					outstream = socket.getOutputStream();

					while (true) {

						synchronized (intro.KEY) {
							if (frame.check == 1) {
								message = frame.s;
								message = "[ " + id + " ] " + message;
								System.out.println("message : " + message);
								if (my_write(outstream,
										message.getBytes("euc-kr"),
										message.length()) == -1) {
									socket.close();
									interrupt();
								}
								;
								frame.check = 0;
							}
						}

						if (!frame.point.equals(beforePoint)) {
							my_write(outstream, frame.point.getBytes("euc-kr"),
									-frame.point.length());
							System.out.println(-frame.point.length());
							beforePoint = frame.point;
						}
					}
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
					interrupt();
				}
			}
		}.start();
	}

	int my_write(OutputStream outstream, byte[] buf, int signedLen)
			throws IOException {
		int num;
		byte[] signedByte;

		System.out.println("buf1 : " + new String(buf));
		num = isHangul(new String(buf).toCharArray());
		signedByte = intTobyte(signedLen + num);
		outstream.write(signedByte);
		System.out.println(signedByte);
		System.out.println("22" + signedByte);
		outstream.write(buf);
		System.out.println("buf2 : " + new String(buf));
		return signedLen;
	}

	int my_read(InputStream instream, byte[] buf) throws IOException {
		byte[] con = new byte[4];
		int signedLen;
		int temp;
		int num;

		if (instream.read(con) == -1)
			return 0;
		signedLen = byteToint(con);
		System.out.println("signedLen : " + signedLen);
		temp = signedLen;
		if (temp < 0)
			temp = -temp;
		instream.read(buf, 0, temp);
		System.out.println("read_buffer : " + new String(buf));

		try {
			if (byteToint(buf) == -1)
				return -1;
		} catch (Exception e) {
			num = isHangul(new String(buf).toCharArray());
			signedLen -= num;
			return signedLen;
		}

		return 0;
	}

	byte[] intTobyte(int signedLen) {
		ByteBuffer buff = ByteBuffer.allocate(Integer.SIZE / 8);

		buff.order(ByteOrder.LITTLE_ENDIAN);
		buff.putInt(signedLen);

		return buff.array();
	}

	int byteToint(byte[] buf) {
		ByteBuffer buff = ByteBuffer.allocate(Integer.SIZE / 8);

		buff.order(ByteOrder.LITTLE_ENDIAN);
		buff.put(buf);
		buff.flip();

		return buff.getInt();
	}

	public String setpoint() {
		String point;
		point = frame.x + "," + frame.y;
		return point;
	}

	public void getpoint(String point) {
		System.out.println(point);
		StringTokenizer tokenizer = new StringTokenizer(point.replaceAll(",",
				" "));

		String s_x1 = tokenizer.nextToken().trim();
		String s_y1 = tokenizer.nextToken().trim();
		String s_x2 = tokenizer.nextToken().trim();
		String s_y2 = tokenizer.nextToken().trim();
		String s_stroke = tokenizer.nextToken().trim();
		String s_color = tokenizer.nextToken().trim();

		int x1 = Integer.parseInt(s_x1);
		int y1 = Integer.parseInt(s_y1);
		int x2 = Integer.parseInt(s_x2);
		int y2 = Integer.parseInt(s_y2);
		int stroke = Integer.parseInt(s_stroke);

		Color c_color = Color.black;
		if (s_color.equals("black"))
			c_color = Color.black;
		else if (s_color.equals("green"))
			c_color = new Color(21, 113, 49);
		else if (s_color.equals("blue"))
			c_color = Color.blue;
		else if (s_color.equals("red"))
			c_color = Color.red;

		if (x1 == -1) {
			frame.can.flag = 1;
			frame.can.repaint();
		} else {
			DrawInfo di = new DrawInfo(x1, y1, x2, y2, stroke, c_color);
			frame.can.vc.add(di);
			frame.can.repaint();
		}
	}

	int isHangul(char[] ch) {
		int num = 0;

		for (int i = 0; i < ch.length; i++) {
			UnicodeBlock block = UnicodeBlock.of(ch[i]);

			if (UnicodeBlock.HANGUL_SYLLABLES == block
					|| UnicodeBlock.HANGUL_JAMO == block
					|| UnicodeBlock.HANGUL_COMPATIBILITY_JAMO == block) {
				num++;
			}
		}

		return num;
	}

	void idupdate(Socket socket) {
		int i, value;
		String name;
		InputStream instream;
		// frame.show();
		try {
			instream = socket.getInputStream();
			byte[] id = new byte[20];
			for (i = 0; i < 4; i++) {
				my_read(instream, id);
				name = new String(id);
				StringTokenizer tokenizer = new StringTokenizer(name);
				name = tokenizer.nextToken().trim();
				// name = new String(id).substring(0, value);
				System.out.println("@ name : " + name + "_");
				// if(name.equals("-1")){
				if (name.charAt(0) == '-' && name.charAt(1) == '0') {
					System.out.println("#inside input");
					break;
				}

				id_list[i] = name;
				if (i == 3) {
					// System.out.println("AAAAAAAAA");
					// timer2.start();
				}
			}
			// id_list[0]="jh";

			for (i = 0; i < 4; i++) {
				frame.table.setValueAt(id_list[i], i, 1);
			}
			first = 1;

		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	class sec_thread extends Thread {
		int sec = 60;

		public void run() {
			String str;
			int cnt = 0;
			OutputStream outstream = null;

			try {
				outstream = socket.getOutputStream();
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}

			frame.can.flag = 1;
			frame.can.repaint();

			for (int i = 0; i < 4; i++) {
				if (frame.table.getValueAt(i, 1).equals(id)) {
					if (frame.table.getValueAt(i, 2).equals("◎")) {
						frame.input.setEnabled(false);
						frame.turn = true;
					} else {
						frame.input.setEnabled(true);
						frame.turn = false;
					}
					break;
				}
			}

			frame.second.setFont(new Font("돋음", Font.BOLD, 30));
			while (true) {

				str = Integer.toString(sec);
				frame.second.setText(str);
				try {
					sleep(1000);
					cnt++;
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					break;
				}
				sec--;
				if (sec == 0)
					break;
			}
			frame.second.setFont(new Font("돋음", Font.BOLD, 20));
			frame.second.setText("Time out!");
			if (cnt == 60) {
				String st = "^";
				for (int i = 0; i < 4; i++)
					frame.table.setValueAt("", i, 2);
				try {
					my_write(outstream, st.getBytes("euc-kr"), st.length());
				} catch (UnsupportedEncodingException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				timer.interrupt();
			}
		}

	}

	class start_thread extends Thread {
		int sec = 5;

		public void run() {
			String str;
			frame.setchat("★★잠시후에 게임이 시작됩니다.★★", 19);

			while (sec > 0) {
				str = Integer.toString(sec);
				String mes = "Count down - " + str;
				// System.out.println(str);
				frame.setchat(mes, mes.length());
				try {
					sleep(1000);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				sec--;
			}
			// timer.start();
		}
	}
}
