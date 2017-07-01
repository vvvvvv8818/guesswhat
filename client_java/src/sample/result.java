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

/* ���� ����� �����ִ� ���̾�α��̴�. 
 * �÷��̾���� ������ ���� ������� ������ �Ű� �����ϰ� �̸� ���̺��� �����־���. 
 */
public class result extends JDialog {
	JTable listTable;
	JLabel label;
	DefaultTableModel model;
	JTable table;
	JScrollPane pane;
	JButton button;

	public result(JTable listTable) {
		// �������� ���ڷ� ���޵Ǿ��� ���̺��� ����
		this.listTable = listTable;

		setTitle("RESULT");
		setBounds(150, 150, 400, 300);
		getContentPane().setLayout(null);

		// "< BANK >" label
		label = new JLabel("< BANK >");
		label.setBounds(165, 20, 60, 10);
		getContentPane().add(label);

		// ���� table
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
		
		// "OK" ��ư
		button = new JButton("OK");
		// ��� ȭ��� ���� ȭ���� ������ ���α׷��� ����ȴ�.
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

	/* �÷��̾���� ������ ���� ������� ������ �Ű� �����ϰ� �̸� ���̺��� ������. */
	void sortList() {
		int[] score = new int[4];
		int bank = 1;
		int[] check = {0, 0, 0, 0};

		//�����ڿ��� ���ڷ� ���� ���̺��� �� �÷��̾���� ������ �����Ͽ� �̸� �����Ѵ�. 
		for (int i = 0; i < 4; i++) {
			score[i] = Integer.parseInt(listTable.getValueAt(i, 3).toString());
			System.out.println("score : "
					+ Integer.parseInt(listTable.getValueAt(i, 3).toString()));
		}
		Arrays.sort(score);

		// ���� ������ ���� �÷��̾ ã�� ��� ���̾�α��� ���̺� ������� �־��ش�.
		// ���� ������ �����ٸ� ���� ������ ������.
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
