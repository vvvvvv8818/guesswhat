package sample;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Stroke;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Vector;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ScrollPaneConstants;
import javax.swing.table.DefaultTableModel;

public class sample extends JFrame {
	Font font = new Font("µ∏¿Ω", Font.BOLD, 20);
	Font font2 = new Font("µ∏¿Ω", Font.BOLD, 17);
	my_canvas can;
	JButton black, red, green, blue, large, small, allclear;
	JTextField input;
	static JTextArea chat;
	JPanel jp2, jp3, jp1;
	JLabel keyword, second;
	JScrollPane scroll;
	JTable table;
	JFrame a;
	static String s;
	int x, y;
	int check = 0;
	// String id;
	boolean turn = false;
	static Object CHATCHAT = new Object();
	String point = "";

	public void show() {
		// this.id = id;
		ImageIcon icon = new ImageIcon("C:\\Users\\±Ë¡÷»Ò\\workspace\\sample\\src\\back.jpg");

		a = new JFrame();
		JPanel back = new JPanel() {
			public void paintComponent(Graphics g) {
				g.drawImage(icon.getImage(), 0, 0, null);
				setOpaque(false);
				super.paintComponent(g);
			}
		};
		back.setBounds(0, 0, 1050, 750);
		back.setLayout(null);
		a.setTitle("Screen");
		a.setSize(1050, 750);
		a.setLayout(null);
		a.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

		jp1 = new JPanel() {
			public void paintComponent(Graphics g) {
				g.drawImage(icon.getImage(), 0, 0, null);
				setOpaque(false);
				super.paintComponent(g);
			}
		};
		jp2 = new JPanel() {
			public void paintComponent(Graphics g) {
				g.drawImage(icon.getImage(), 0, 0, null);
				setOpaque(false);
				super.paintComponent(g);
			}
		};
		jp3 = new JPanel() {
			public void paintComponent(Graphics g) {
				g.drawImage(icon.getImage(), 0, 0, null);
				setOpaque(false);
				super.paintComponent(g);
			}
		};
		// jp3.setBackground(Color.white);
		JPanel jp4 = new JPanel() {
			public void paintComponent(Graphics g) {
				g.drawImage(icon.getImage(), 0, 0, null);
				setOpaque(false);
				super.paintComponent(g);
			}
		};
		JPanel jp5 = new JPanel();

		jp5.setBackground(Color.BLACK);
		// jp3.setBackground(Color.BLUE);

		jp1.setBounds(10, 0, 400, 250);
		jp2.setBounds(10, 250, 400, 450);
		jp3.setBounds(420, 0, 600, 60);
		jp4.setBounds(420, 60, 600, 40);
		jp5.setBounds(420, 100, 600, 600);

		JP1(jp1);
		JP2(jp2);
		JP3(jp3);
		JP4(jp4);
		JP5(jp5);

		a.add(jp1);
		a.add(jp2);
		a.add(jp3);
		a.add(jp4);
		a.add(jp5);

		a.add(back);
		back.add(jp1);
		back.add(jp2);
		back.add(jp3);
		back.add(jp4);
		back.add(jp5);

		a.setVisible(true);
	}

	void JP1(JPanel jp1) {
		jp1.setLayout(null);

		JLabel lb = new JLabel("< Member List >");
		lb.setOpaque(true);
		lb.setBounds(120, 0, 400, 50);
		lb.setFont(font);
		lb.setOpaque(false);
		// ∏ÆΩ∫∆Æ≈◊¿Ã∫Ì
		String ID[] = { "Number", "Member's ID", "Tern", "Score" };
		Object data[][] = { { 1, "", "", 0 }, { 2, "", "", 0 },
				{ 3, "", "", 0 }, { 4, "", "", 0 } };
		DefaultTableModel defaultmodel = new DefaultTableModel(data, ID);
		table = new JTable(defaultmodel);

		table.setPreferredScrollableViewportSize(new Dimension(450, 00));
		table.setRowHeight(40);
		table.setIntercellSpacing(new Dimension(10, 5));
		table.setShowGrid(false);
		table.setShowHorizontalLines(true);
		table.getColumn("Number").setPreferredWidth(30);
		table.getColumn("Member's ID").setPreferredWidth(100);
		table.getColumn("Tern").setPreferredWidth(40);
		table.getColumn("Score").setPreferredWidth(40);
		table.setBounds(0, 50, 400, 160);
		table.setFont(font2);

		// table.setValueAt("jh", 0, 1);
		jp1.add(lb);
		jp1.add(table);
	}

	void JP2(JPanel jp2) {
		jp2.setLayout(null);

		JLabel lb = new JLabel("<Chatting Screen>");
		lb.setFont(font);
		lb.setBounds(100, 0, 400, 50);

		chat = new JTextArea();

		chat.setLineWrap(true);
		chat.setEditable(false);
		chat.setColumns(30);
		// chat.setRows(15);
		chat.setText("°⁄°⁄ ¿‘¿Âº∫∞¯ °⁄°⁄" + "\n");
		chat.setFont(new Font("±º∏≤", Font.PLAIN, 19));
		// chat.setBounds(0, 50, 400, 350);
		chat.setCaretPosition(chat.getDocument().getLength());
		scroll = new JScrollPane(chat);

		scroll.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
		scroll.setBounds(0, 50, 400, 350);

		JLabel lb2 = new JLabel("Input :");
		lb2.setFont(new Font("µ∏¿Ω", Font.BOLD, 20));
		lb2.setBounds(40, 400, 90, 50);

		input = new JTextField(20);
		input.addActionListener(new Listener());
		input.setFont(font2);
		input.setBounds(120, 405, 220, 40);

		jp2.add(scroll);
		jp2.add(lb);
		// jp2.add(chat);
		jp2.add(lb2);
		jp2.add(input);
	}

	void JP3(JPanel jp3) {
		jp3.setLayout(null);

		drawPanel dp = new drawPanel();
		// dp.paint(g);

		keyword = new JLabel("");
		keyword.setFont(new Font("µ∏¿Ω", Font.BOLD, 30));
		keyword.setBounds(230, 5, 200, 50);

		second = new JLabel();
		second.setFont(new Font("µ∏¿Ω", Font.BOLD, 30));
		second.setBounds(500, 5, 100, 50);
		second.setForeground(Color.red);
		// second.setBackground(Color.DARK_GRAY);

		jp3.add(keyword);
		jp3.add(second);
		jp3.add(dp);
	}

	void JP4(JPanel jp4) {

		Font f = new Font("±º∏≤", Font.BOLD, 12);

		jp4.add(black = new JButton("BLACK"));
		jp4.add(red = new JButton("RED"));
		jp4.add(green = new JButton("GREEN"));
		jp4.add(blue = new JButton("BLUE"));
		jp4.add(large = new JButton("Size Up"));
		jp4.add(small = new JButton("Size Down"));
		jp4.add(allclear = new JButton("Clear"));

		black.setBackground(Color.black);
		red.setBackground(Color.red);
		green.setBackground(new Color(21, 113, 49));
		blue.setBackground(Color.blue);

		black.setForeground(Color.white);
		blue.setForeground(Color.white);
		black.setFont(f);
		red.setFont(f);
		green.setFont(f);
		blue.setFont(f);
		large.setFont(f);
		small.setFont(f);
		allclear.setFont(f);

		large.setBackground(new Color(245, 239, 207));
		small.setBackground(new Color(245, 239, 207));
		allclear.setBackground(new Color(245, 239, 207));

	}

	void JP5(JPanel jp5) {
		can = new my_canvas();
		can.setSize(550, 580);
		can.setBackground(Color.white);
		jp5.add(can);

		MyHandler my = new MyHandler();
		buttonHanbler bh = new buttonHanbler();
		black.addActionListener(bh);
		red.addActionListener(bh);
		green.addActionListener(bh);
		blue.addActionListener(bh);
		large.addActionListener(bh);
		small.addActionListener(bh);
		allclear.addActionListener(bh);
		can.addMouseMotionListener(my);
		can.addMouseListener(my);

		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				System.exit(0);
			}
		});
	}

	public void setchat(String message, int value) {
		synchronized (CHATCHAT) {
			message = message.substring(0, value);
			chat.append(message + "\n");
		}
		scroll.getVerticalScrollBar().setValue(
				scroll.getVerticalScrollBar().getMaximum());
	}

	class buttonHanbler implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			Object o = e.getSource();
			can.flag = 0;// flag∏¶ 0¿∏∑Œ ¡÷æÓ ±◊∏Æ±‚∞° ±‚∫ª¿€æ˜¿Ã µ«∞‘ «—¥Ÿ.
			if (o == red) {
				can.cr = Color.red;
			} else if (o == black) {
				can.cr = Color.BLACK;
			} else if (o == green) {
				can.cr = new Color(21, 113, 49);
			} else if (o == blue) {
				can.cr = Color.blue;
			} else if (o == large) {
				// can.w++;
				// can.h++;
				can.stroke++;
			} else if (o == small) {
				if (can.stroke > 3) {
					// can.w--;
					// can.h--;
					can.stroke--;
				}
			} else if (o == allclear && turn == true) {
				can.flag = 1;
				can.repaint();
				point = setpoint(-1, -1, -1, -1, -1, Color.black);
			}
		}
	}

	class MyHandler implements MouseListener, MouseMotionListener {

		@Override
		public void mouseClicked(MouseEvent e) {
			// TODO Auto-generated method stub

		}

		@Override
		public void mouseEntered(MouseEvent e) {
			// TODO Auto-generated method stub

		}

		@Override
		public void mouseExited(MouseEvent e) {
			// TODO Auto-generated method stub

		}

		@Override
		public void mousePressed(MouseEvent e) {
			// TODO Auto-generated method stub
			if (turn == true) {
				can.start_x = e.getX();
				can.start_y = e.getY();
			}
		}

		@Override
		public void mouseReleased(MouseEvent e) {
			// TODO Auto-generated method stub
			if (turn == true) {
				can.x_x = e.getX();
				can.y_y = e.getY();
				DrawInfo di = new DrawInfo(can.start_x, can.start_y, can.x_x,
						can.y_y, can.stroke, can.cr);
				point = setpoint(can.start_x, can.start_y, can.x_x, can.y_y,
						can.stroke, can.cr);
				can.vc.add(di);
				can.repaint();
			}
		}

		@Override
		public void mouseDragged(MouseEvent e) {
			// TODO Auto-generated method stub
			if (turn == true) {
				can.x_x = e.getX();
				can.y_y = e.getY();
				DrawInfo di = new DrawInfo(can.start_x, can.start_y, can.x_x,
						can.y_y, can.stroke, can.cr);
				point = setpoint(can.start_x, can.start_y, can.x_x, can.y_y,
						can.stroke, can.cr);
				can.vc.add(di);
				can.start_x = can.x_x;
				can.start_y = can.y_y;
				can.repaint();
			}
		}

		@Override
		public void mouseMoved(MouseEvent e) {
			// TODO Auto-generated method stub

		}
	}

	class Listener implements ActionListener {
		public void actionPerformed(ActionEvent event) {
			if (event.getSource() == input) {
				synchronized (intro.KEY) {
					check = 1;
					JTextField tf = (JTextField) event.getSource();
					s = tf.getText();
					input.setText("");
				}
			}
		}

	}

	class drawPanel extends JPanel {
		public void paint(Graphics g) {
			g.drawRect(500, 3, 50, 50);
		}
	}

	public String setpoint(int x1, int y1, int x2, int y2, int stroke,
			Color color) {
		String point;
		String s_color = "";

		if (color == Color.black)
			s_color = "black";
		else if (color == Color.blue)
			s_color = "blue";
		else if (color == Color.red)
			s_color = "red";
		else
			s_color = "green";

		point = x1 + "," + y1 + "," + x2 + "," + y2 + "," + stroke + ","
				+ s_color;

		return point;
	}

	public void getpoint(String point) {
		int loca = point.indexOf(',');
		String stx, sty;
		stx = point.substring(0, loca);
		sty = point.substring(loca + 1);
		x = Integer.parseInt(stx);
		y = Integer.parseInt(sty);
	}
}