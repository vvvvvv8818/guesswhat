package sample;

import java.awt.BasicStroke;
import java.awt.Canvas;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.util.Vector;

public class my_canvas extends Canvas {
	int x = -13, y = -13, w = 7, h = 7;
	Color cr = Color.black;
	int flag = 0;
	int start_x = 0, start_y = 0, x_x = 0, y_y = 0;
	Vector vc = new Vector();
	int stroke = 3;

	public void paint(Graphics g) {

		if (flag == 0) {
			// 1. 그리기
			g.setColor(cr);
			// g.fillOval(x, y, w, h);
			Graphics2D g2 = (Graphics2D) g;
			g2.setStroke(new BasicStroke(stroke, BasicStroke.CAP_ROUND, 0));
			g.drawLine(start_x, start_y, x_x, y_y);

			System.out.println(vc.size());
			for (int i = 0; i < vc.size(); i++) {
				DrawInfo di = (DrawInfo) vc.elementAt(i);
				g2 = (Graphics2D) g;
				g2.setStroke(new BasicStroke(di.getStroke(),
						BasicStroke.CAP_ROUND, 0));
				g.setColor(di.getColor());
				g.drawLine(di.getStart_x(), di.getStart_y(), di.getX_x(),
						di.getY_y());
			}
		} else if (flag == 1) {
			// 2. 모두 지우기..
			g.clearRect(0, 0, 700, 680);
			flag = 0;
			vc.clear();
		}
	}

	public void update(Graphics g) {
		paint(g);
	}

}
