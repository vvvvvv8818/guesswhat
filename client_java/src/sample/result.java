package sample;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Arrays;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.SwingConstants;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;

/* 게임 결과를 보여주는 다이얼로그이다. 
 * 플레이어들의 점수를 높은 순서대로 순위를 매겨 나열하고 이를 테이블이 보여주었다. 
 */
public class result extends JDialog {
	JTable listTable;
	JLabel label;
	DefaultTableModel model;
	JTable table;
	JScrollPane pane;
	JButton button;

	public result(JTable listTable) {
		// 생성자의 인자로 전달되어진 테이블을 저장
		this.listTable = listTable;

		setTitle("RESULT");
		setBounds(150, 150, 400, 300);
		getContentPane().setLayout(null);

		// "< BANK >" label
		label = new JLabel("< BANK >");
		label.setBounds(165, 20, 60, 10);
		getContentPane().add(label);

		// 순위 table
		String[] field = { "Bank", "Member's ID", "Score" };
		model = new DefaultTableModel(field, 0);
		sortList();
		
		table = new JTable(model);
		DefaultTableCellRenderer render = new DefaultTableCellRenderer();
		render.setHorizontalAlignment(SwingConstants.CENTER);
		for(int i = 0; i < 3; i++){
			if(i == 0)
				table.getColumnModel().getColumn(0).setPreferredWidth(60);
			else{
				table.getColumnModel().getColumn(i).setPreferredWidth(130);
			}
			table.setRowHeight(i, 30);
			table.getColumnModel().getColumn(i).setCellRenderer(render);
		}
		table.setRowHeight(3, 30);
		
		pane = new JScrollPane(table);
		pane.setBounds(30, 50, 320, 143);
		getContentPane().add(pane);
		
		// "OK" 버튼
		button = new JButton("OK");
		// 결과 화면과 게임 화면이 닫히고 프로그램이 종료된다.
		button.addActionListener(new ActionListener() {
			
			@Override
			public void actionPerformed(ActionEvent e) {
				// TODO Auto-generated method stub
				//dispose();
				System.exit(0);
			}
		});
		button.setBounds(150, 210, 80, 40);
		getContentPane().add(button);
	}

	/* 플레이어들의 점수를 높은 순서대로 순위를 매겨 나열하고 이를 테이블이 보여줌. */
	void sortList() {
		int[] score = new int[4];
		int bank = 1;
		int[] check = {0, 0, 0, 0};

		//생성자에서 인자로 받은 테이블에서 각 플레이어들의 점수를 저장하여 이를 정렬한다. 
		for (int i = 0; i < 4; i++) {
			score[i] = Integer.parseInt(listTable.getValueAt(i, 3).toString());
			System.out.println("score : "
					+ Integer.parseInt(listTable.getValueAt(i, 3).toString()));
		}
		Arrays.sort(score);

		// 같은 점수를 가진 플레이어를 찾아 결과 다이얼로그의 테이블에 순서대로 넣어준다.
		// 같은 점수를 가진다면 같은 순위를 가진다.
		for (int i = 3; i >= 0; i--) {
			for (int j = 0; j < 4; j++) {
				if(score[i] == Integer.parseInt(listTable.getValueAt(j, 3).toString()) && check[j] == 0){
					if(i!=3 && score[i] != score[i+1]){
						bank = 4-i;
					}
					String[] item = {Integer.toString(bank), listTable.getValueAt(j, 1).toString(), listTable.getValueAt(j, 3).toString()};
					model.addRow(item);
					check[j] = 1;
					break;
				}
			}
		}
	}
}
