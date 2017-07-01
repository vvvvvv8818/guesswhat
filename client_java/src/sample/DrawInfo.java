package sample;

import java.awt.Color;
import java.io.Serializable;

public class DrawInfo implements Serializable{
	private int start_x;
	private int start_y;
	private int x_x;
	private int y_y;
	private int stroke;
	private Color color;
	
	public DrawInfo(int start_x, int start_y, int x_x, int y_y, int stroke, Color color){
		this.start_x = start_x;
		this.start_y = start_y;
		this.x_x = x_x;
		this.y_y = y_y;
		this.stroke = stroke;
		this.color = color;
	}

	public int getStart_x() {
		return start_x;
	}

	public void setStart_x(int start_x) {
		this.start_x = start_x;
	}

	public int getStart_y() {
		return start_y;
	}

	public void setStart_y(int start_y) {
		this.start_y = start_y;
	}

	public int getX_x() {
		return x_x;
	}

	public void setX_x(int x_x) {
		this.x_x = x_x;
	}

	public int getY_y() {
		return y_y;
	}

	public void setY_y(int y_y) {
		this.y_y = y_y;
	}

	public int getStroke() {
		return stroke;
	}

	public void setStroke(int stroke) {
		this.stroke = stroke;
	}

	public Color getColor() {
		return color;
	}

	public void setColor(Color color) {
		this.color = color;
	}
}
