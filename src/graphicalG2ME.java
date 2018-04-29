import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.EmptyBorder;
import javax.swing.UIManager.*;
import javax.swing.event.*;
import javax.swing.text.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.prefs.*;

import static javax.swing.WindowConstants.EXIT_ON_CLOSE;

public class graphicalG2ME {

	/* Prefs variables */
	private static final String G2ME_DIR="/home/me/G2MEGit/";
	private static final String G2ME_PLAYER_DIR="/home/me/G2MEGit/.players/";
	/* Prefs defaults */
	static String G2ME_DIR_DEFAULT="/home/me/G2MEGit/";
	static String G2ME_PLAYER_DIR_DEFAULT="/home/me/G2MEGit/.players/";

	final int ELEMENT_SPACING = 5;
	final int TEXTFIELD_HEIGHT = 32;
	final int CHECKBOX_HEIGHT = 24;
	String playerInformationCurrentFlag = "h";
	String playerInformationLastName = "";
	int playerInformationSearchLastLength = 0;
	String playerRecordsLastName = "";
	int playerRecordsSearchLastLength = 0;


	/* Aliased GUI classes */
	public class JAliasedTextField extends JTextField {

		public JAliasedTextField() { super(); }
		public JAliasedTextField(String s) { super(s); }

		@Override
		public void paintComponent(Graphics g) {
			Graphics2D graphics2d = (Graphics2D) g;
			graphics2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
			RenderingHints.VALUE_ANTIALIAS_ON);
			super.paintComponent(g);
		}
	}

	public class JAliasedTextArea extends JTextArea {

		public JAliasedTextArea() { super(); }
		public JAliasedTextArea(String s) { super(s); }

		@Override
		public void paintComponent(Graphics g) {
			Graphics2D graphics2d = (Graphics2D) g;
			graphics2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
			RenderingHints.VALUE_ANTIALIAS_ON);
			super.paintComponent(g);
		}
	}

	public class JAliasedList extends JList {

		public JAliasedList() { super(); }

		@Override
		public void paintComponent(Graphics g) {
			Graphics2D graphics2d = (Graphics2D) g;
			graphics2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
			RenderingHints.VALUE_ANTIALIAS_ON);
			super.paintComponent(g);
		}
	}

	public class JAliasedButton extends JButton {
		public JAliasedButton(String s) {
			super(s);
		}

		@Override
		public void paintComponent(Graphics g) {
			Graphics2D graphics2d = (Graphics2D) g;
			graphics2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
			RenderingHints.VALUE_ANTIALIAS_ON);
			super.paintComponent(g);
		}
	}

	public class JAliasedCheckBox extends JCheckBox {
		public JAliasedCheckBox(String s) {
			super(s);
		}

		@Override
		public void paintComponent(Graphics g) {
			Graphics2D graphics2d = (Graphics2D) g;
			graphics2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
			RenderingHints.VALUE_ANTIALIAS_ON);
			super.paintComponent(g);
		}
	}

	public class JAliasedRadioButton extends JRadioButton {
		public JAliasedRadioButton(String s) {
			super(s);
		}

		@Override
		public void paintComponent(Graphics g) {
			Graphics2D graphics2d = (Graphics2D) g;
			graphics2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
			RenderingHints.VALUE_ANTIALIAS_ON);
			super.paintComponent(g);
		}
	}
	/* End of Aliased GUI classes */

	public static void main(String[] args) {
		new graphicalG2ME();
	}

	public int DisplayCommandResultsInJTextArea(String command, JTextArea t, boolean stack) {
		try {
			System.out.println("running \"" + command + "\"");
			Runtime rt = Runtime.getRuntime();
			Process pr = rt.exec(command);

			BufferedReader input =
				new BufferedReader(new InputStreamReader(pr.getInputStream()));

			String line;
			boolean not_first = false;
			if (stack) not_first = !t.getText().equals("");

			if (stack == false) t.setText("");
			while ((line = input.readLine()) != null) {
				if (not_first) t.setText(t.getText() + "\n" + line);
				else t.setText(t.getText() + line);
				not_first = true;
			}

			int ret = pr.waitFor();
			return ret;

		} catch(Exception e) {
		    System.out.println(e.toString());
		    e.printStackTrace();
			return -1;
		}
	}

	public void UpdateJListToFilesInDir(JList l, String PlayerDirPath) {
		File PlayerDirectory = new File(PlayerDirPath);
		if (PlayerDirectory != null && PlayerDirectory.isDirectory()) {
			File[] listOfFiles = PlayerDirectory.listFiles();
			Arrays.sort(listOfFiles);
			String listOfFileNames[] = new String[listOfFiles.length];
			for (int i = 0; i < listOfFiles.length; i++) {
				listOfFileNames[i] = listOfFiles[i].getName();
			}

			l.setListData(listOfFileNames);
		} else {
			System.err.println("Saved G2ME Player Directory file path cannot be opened");
		}
	}

	public void UpdateJTextAreaToFlag(JTextArea t, String playerName, boolean verbose, String flag) {
		Preferences prefs = Preferences.userRoot().node(this.getClass().getName());
		String flags = "-n" + flag;
		int ret = 0;

		if (verbose) flags = "-nv" + flag;

		ret = DisplayCommandResultsInJTextArea(
			"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
			" " + flags + " " + playerName, t, false);

		if (ret != 0) {
			System.err.println("An error occurred running \"" +
				"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
				" " + flags + " " + playerName + "\"");
		}
	}

	public void UpdateJListToSearchString(JList l, String s) {
		Preferences prefs = Preferences.userRoot().node(this.getClass().getName());
		File PlayerDirectory = new File(prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT));
		File[] listOfFiles = PlayerDirectory.listFiles();
		Arrays.sort(listOfFiles);
		ArrayList<String> items = new ArrayList<>();
		for (int i = 0; i < listOfFiles.length; i++) {
			boolean mismatch = false;
			// This file can only match the search if its length is shorter
			// than or equal to the search
			if (s.length() <= listOfFiles[i].getName().length()) {
				for (int j = 0; j < s.length(); j++) {

					if (s.length() > 0) {
						if (listOfFiles[i].getName().charAt(j) != s.charAt(j)) {
							mismatch = true;
							break;
						}
					}
				}
				if (mismatch == false) items.add(listOfFiles[i].getName());
			}
		}
		l.setListData(items.toArray());
	}

	public graphicalG2ME() {
		/* Load preferences */
		Preferences prefs = Preferences.userRoot().node(this.getClass().getName());

		/* Try to set Look and Feel to GTK if available */
		try {
			for (LookAndFeelInfo info : UIManager.getInstalledLookAndFeels()) {
				if ("GTK+".equals(info.getName())) {
					UIManager.setLookAndFeel(info.getClassName());
					break;
				}
			}
		} catch (Exception e) {
			/* Otherwise use the system look and feel */
			try {
				UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			} catch (Exception e2) {
				System.err.println("Error: could not find system look and feel. Exiting...");
				System.exit(1);
			}
		}

		JPanel tabSettings = new JPanel(null);
		JPanel tabPowerRankings = new JPanel(null);
		JPanel tabPlayerInformation = new JPanel(null);
		JPanel tabRunBrackets = new JPanel(null);
		/* Configure tabs */
		tabSettings.setPreferredSize(tabSettings.getPreferredSize());
		tabPowerRankings.setPreferredSize(tabPowerRankings.getPreferredSize());
		tabPlayerInformation.setPreferredSize(tabPlayerInformation.getPreferredSize());
		/* Validate tabs */
		tabSettings.validate();
		tabPowerRankings.validate();
		tabPlayerInformation.validate();

		/* Configure Settings Tab */
		JLabel SettingsG2MEDirLabel = new JLabel("G2ME Directory file path");
		JAliasedTextField SettingsG2MEDirTextField = new JAliasedTextField();
		JLabel SettingsG2MEPlayerDirLabel = new JLabel("G2ME Players-Directory file path");
		JAliasedTextField SettingsG2MEPlayerDirTextField = new JAliasedTextField();
		JAliasedButton SettingsSaveButton = new JAliasedButton("Save");
		SettingsSaveButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				prefs.put(G2ME_DIR, SettingsG2MEDirTextField.getText());
				prefs.put(G2ME_PLAYER_DIR, SettingsG2MEPlayerDirTextField.getText());
				File G2MEDirectory = new File(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				File PlayerDirectory = new File(prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT));
				if (!(G2MEDirectory != null && G2MEDirectory.isDirectory())) {
					SettingsG2MEDirTextField.setForeground(Color.red);
				} else {
					SettingsG2MEDirTextField.setForeground(Color.green);
				}
				if (!(PlayerDirectory != null && PlayerDirectory.isDirectory())) {
					SettingsG2MEPlayerDirTextField.setForeground(Color.red);
				} else {
					SettingsG2MEPlayerDirTextField.setForeground(Color.green);
				}
			}
		});
		/* Set default text for the 2 text fields */
		SettingsG2MEDirTextField.setText(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
		SettingsG2MEPlayerDirTextField.setText(prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT));
		/* Check that the 2 file paths lead to existing directories */
		File G2MEDirectory = new File(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
		File PlayerDirectory = new File(prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT));
		if (!(G2MEDirectory != null && G2MEDirectory.isDirectory())) {
			SettingsG2MEDirTextField.setForeground(Color.red);
		} else {
			SettingsG2MEDirTextField.setForeground(Color.green);
		}
		if (!(PlayerDirectory != null && PlayerDirectory.isDirectory())) {
			SettingsG2MEPlayerDirTextField.setForeground(Color.red);
		} else {
			SettingsG2MEPlayerDirTextField.setForeground(Color.green);
		}
		/* Use Box Layout for this tab */
		tabSettings.setLayout(new BoxLayout(tabSettings, BoxLayout.Y_AXIS));
		/* Layout settings for the tab */
		SettingsG2MEDirTextField.setMinimumSize(new Dimension(60, TEXTFIELD_HEIGHT));
		SettingsG2MEDirTextField.setPreferredSize(new Dimension(60, TEXTFIELD_HEIGHT));
		SettingsG2MEDirTextField.setMaximumSize(new Dimension(Short.MAX_VALUE, TEXTFIELD_HEIGHT));
		SettingsG2MEPlayerDirTextField.setMinimumSize(new Dimension(60, TEXTFIELD_HEIGHT));
		SettingsG2MEPlayerDirTextField.setPreferredSize(new Dimension(60, TEXTFIELD_HEIGHT));
		SettingsG2MEPlayerDirTextField.setMaximumSize(new Dimension(Short.MAX_VALUE, TEXTFIELD_HEIGHT));
		SettingsSaveButton.setMinimumSize(new Dimension(50, TEXTFIELD_HEIGHT));
		SettingsSaveButton.setPreferredSize(new Dimension(70, TEXTFIELD_HEIGHT));
		SettingsSaveButton.setMaximumSize(new Dimension(90, TEXTFIELD_HEIGHT));
		SettingsG2MEDirLabel.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsG2MEDirTextField.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsG2MEPlayerDirLabel.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsG2MEPlayerDirTextField.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsSaveButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		/* Add all the elements to the tab (with spacing) */
		tabSettings.setBorder(new EmptyBorder(ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING));
		tabSettings.add(SettingsG2MEDirLabel);
		tabSettings.add(Box.createRigidArea(new Dimension(0,ELEMENT_SPACING)));
		tabSettings.add(SettingsG2MEDirTextField);
		tabSettings.add(Box.createRigidArea(new Dimension(0,ELEMENT_SPACING)));
		tabSettings.add(SettingsG2MEPlayerDirLabel);
		tabSettings.add(Box.createRigidArea(new Dimension(0,ELEMENT_SPACING)));
		tabSettings.add(SettingsG2MEPlayerDirTextField);
		tabSettings.add(Box.createRigidArea(new Dimension(0,ELEMENT_SPACING)));
		tabSettings.add(SettingsSaveButton);

		/* Configure Power Rankings Tab */
		JPanel PowerRankingsControlBar = new JPanel();
		PowerRankingsControlBar.setLayout(new BoxLayout(PowerRankingsControlBar, BoxLayout.X_AXIS));
		JAliasedButton PowerRankingsGenPRButton = new JAliasedButton("Generate Power Rankings");
		JAliasedButton PowerRankingsGenPRVerboseButton = new JAliasedButton("Generate Power Rankings (Verbose)");
		JAliasedTextField PowerRankingsFilterFileTextField = new JAliasedTextField();
		JAliasedButton PowerRankingsSaveButton = new JAliasedButton("Save As...");
		JAliasedTextArea PowerRankingsTextDialog = new JAliasedTextArea();
		JScrollPane PowerRankingsTextDialogScroll = new JScrollPane(PowerRankingsTextDialog);

		PowerRankingsTextDialog.setFont(new Font("monospaced", Font.PLAIN, 12));

		PowerRankingsGenPRButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				int ret = 0;
				if (PowerRankingsFilterFileTextField.getText().equals("")) {
					ret = DisplayCommandResultsInJTextArea(
						"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) + " -O",
						PowerRankingsTextDialog, false);
					if (ret != 0) {
						System.err.println("An error occurred running \"" +
							"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) + " -O" + "\"");
					}
				} else {
					ret = DisplayCommandResultsInJTextArea(
						"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) + " -p "
						+ PowerRankingsFilterFileTextField.getText() + " -O", PowerRankingsTextDialog, false);
					if (ret != 0) {
						System.err.println("An error occurred running \"" +
							"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) + " -p "
							+ PowerRankingsFilterFileTextField.getText() + " -O" + "\"");
					}
				}
			}
		});
		PowerRankingsGenPRVerboseButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				int ret = 0;
				if (PowerRankingsFilterFileTextField.getText().equals("")) {
					ret = DisplayCommandResultsInJTextArea(
							"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) + " -vO",
							PowerRankingsTextDialog, false);
					if (ret != 0) {
						System.err.println("An error occurred running \"" +
							"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) + " -vO" + "\"");
					}
				} else {
					ret = DisplayCommandResultsInJTextArea(
						"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) + " -p "
						+ PowerRankingsFilterFileTextField.getText() + " -vO", PowerRankingsTextDialog, false);
					if (ret != 0) {
						System.err.println("An error occurred running \"" +
							"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) + " -p "
							+ PowerRankingsFilterFileTextField.getText() + " -vO" + "\"");
					}
				}
			}
		});
		PowerRankingsSaveButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Save Power Rankings As...", FileDialog.SAVE);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && DestinationFile != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						BufferedWriter writer = Files.newBufferedWriter(Paths.get(DestinationFile.getAbsolutePath()));
						writer.write(PowerRankingsTextDialog.getText());
						writer.close();
						System.out.println("Wrote to file");
					} catch (Exception e1) {
						e1.printStackTrace();
					}
				}
			}
		});
		/* Use Box Layout for this tab */
		tabPowerRankings.setLayout(new BoxLayout(tabPowerRankings, BoxLayout.Y_AXIS));
		/* Layout settings for the tab */
		PowerRankingsGenPRButton.setMinimumSize(new Dimension(70, TEXTFIELD_HEIGHT));
		PowerRankingsGenPRButton.setPreferredSize(new Dimension(180, TEXTFIELD_HEIGHT));
		PowerRankingsGenPRButton.setMaximumSize(new Dimension(210, TEXTFIELD_HEIGHT));
		PowerRankingsGenPRButton.setToolTipText("Generate Power Rankings");
		PowerRankingsGenPRVerboseButton.setMinimumSize(new Dimension(70, TEXTFIELD_HEIGHT));
		PowerRankingsGenPRVerboseButton.setPreferredSize(new Dimension(280, TEXTFIELD_HEIGHT));
		PowerRankingsGenPRVerboseButton.setMaximumSize(new Dimension(300, TEXTFIELD_HEIGHT));
		PowerRankingsGenPRVerboseButton.setToolTipText("Generate Power Rankings (Verbose)");
		PowerRankingsFilterFileTextField.setMinimumSize(new Dimension(70, TEXTFIELD_HEIGHT));
		PowerRankingsFilterFileTextField.setPreferredSize(new Dimension(160, TEXTFIELD_HEIGHT));
		PowerRankingsFilterFileTextField.setMaximumSize(new Dimension(300, TEXTFIELD_HEIGHT));
		PowerRankingsFilterFileTextField.setToolTipText("File path for a filter file");
		PowerRankingsSaveButton.setMinimumSize(new Dimension(90, TEXTFIELD_HEIGHT));
		PowerRankingsSaveButton.setPreferredSize(new Dimension(90, TEXTFIELD_HEIGHT));
		PowerRankingsSaveButton.setMaximumSize(new Dimension(100, TEXTFIELD_HEIGHT));
		PowerRankingsSaveButton.setToolTipText("Save As...");
		PowerRankingsTextDialogScroll.setMinimumSize(new Dimension(100, 300));
		PowerRankingsTextDialogScroll.setPreferredSize(new Dimension(120, 500));
		PowerRankingsTextDialogScroll.setMaximumSize(new Dimension(Short.MAX_VALUE, Short.MAX_VALUE));
		/* Add all elements in the control bar to the control bar panel */
		PowerRankingsControlBar.add(PowerRankingsGenPRButton);
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		PowerRankingsControlBar.add(PowerRankingsGenPRVerboseButton);
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		PowerRankingsControlBar.add(PowerRankingsFilterFileTextField);
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		PowerRankingsControlBar.add(PowerRankingsSaveButton);
		/* Add all the elements to the tab (with spacing) */
		tabPowerRankings.setBorder(new EmptyBorder(ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING));
		tabPowerRankings.add(PowerRankingsControlBar);
		tabPowerRankings.add(Box.createRigidArea(new Dimension(0,ELEMENT_SPACING)));
		tabPowerRankings.add(PowerRankingsTextDialogScroll);

		/* Configure Player Information Tab */
		JPanel PlayerInformationControlBar = new JPanel();
		PlayerInformationControlBar.setLayout(new BoxLayout(PlayerInformationControlBar, BoxLayout.Y_AXIS));
		JAliasedButton PlayerInformationRefreshButton = new JAliasedButton("Refresh");
		JAliasedCheckBox PlayerInformationVerboseCheckBox = new JAliasedCheckBox("Verbose");
		JAliasedTextField PlayerInformationSearchTextField = new JAliasedTextField();
		JAliasedList PlayerInformationPlayerList = new JAliasedList();
		JScrollPane PlayerInformationPlayerListScroll = new JScrollPane(PlayerInformationPlayerList);
		JAliasedTextArea PlayerInformationTextDialog = new JAliasedTextArea();
		JScrollPane PlayerInformationTextDialogScroll = new JScrollPane(PlayerInformationTextDialog);

		JAliasedRadioButton PlayerInformationHistoryButton = new JAliasedRadioButton("Outcome History");
		PlayerInformationHistoryButton.setSelected(true);
		PlayerInformationHistoryButton.setToolTipText("Opponent, Date, Tournament and Glicko2 Data After Every Outcome (Set/Game)");
		JAliasedRadioButton PlayerInformationRecordsButton = new JAliasedRadioButton("Records/Head-to-Heads");
		PlayerInformationRecordsButton.setToolTipText("Wins, Ties, Losses Against All Players Played");
		JAliasedRadioButton PlayerInformationEventsAttendedButton = new JAliasedRadioButton("Events Attended");
		PlayerInformationEventsAttendedButton.setToolTipText("Names of All Events Attended");
		JAliasedRadioButton PlayerInformationNumOutcomesButton = new JAliasedRadioButton("Number of Outcomes (Sets/Games) Played");
		PlayerInformationNumOutcomesButton.setToolTipText("Number of Outcomes (Sets/Games) Played");

		PlayerInformationHistoryButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				PlayerInformationVerboseCheckBox.setEnabled(true);
				playerInformationCurrentFlag = "h";
			}
		});
		PlayerInformationRecordsButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				PlayerInformationVerboseCheckBox.setEnabled(false);
				playerInformationCurrentFlag = "R";
			}
		});
		PlayerInformationEventsAttendedButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				PlayerInformationVerboseCheckBox.setEnabled(false);
				playerInformationCurrentFlag = "A";
			}
		});
		PlayerInformationNumOutcomesButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				PlayerInformationVerboseCheckBox.setEnabled(false);
				playerInformationCurrentFlag = "c";
			}
		});
		ButtonGroup PlayerInformationButtonGroup = new ButtonGroup();
		PlayerInformationButtonGroup.add(PlayerInformationHistoryButton);
		PlayerInformationButtonGroup.add(PlayerInformationRecordsButton);
		PlayerInformationButtonGroup.add(PlayerInformationEventsAttendedButton);
		PlayerInformationButtonGroup.add(PlayerInformationNumOutcomesButton);

		PlayerInformationTextDialog.setFont(new Font("monospaced", Font.PLAIN, 12));
		KeyListener PlayerInformationSearchKeyListener = new KeyListener() {
			public void keyReleased(KeyEvent keyEvent) {
				String searchText = PlayerInformationSearchTextField.getText();
				UpdateJListToSearchString(PlayerInformationPlayerList, searchText);
				playerInformationSearchLastLength = searchText.length();
			}

			public void keyPressed(KeyEvent keyEvent) {}
			public void keyTyped(KeyEvent keyEvent) {}
		};
		PlayerInformationSearchTextField.addKeyListener(PlayerInformationSearchKeyListener);
		PlayerInformationSearchTextField.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				/* Display data for player of [first name in JList] */
				if (PlayerInformationPlayerList.getFirstVisibleIndex() != -1) {
					String newValue =
						PlayerInformationPlayerList.getModel().getElementAt(
						PlayerInformationPlayerList.getFirstVisibleIndex()).toString();
					if (newValue != null) {
						playerInformationLastName = newValue;
						/* Update player information currently in dialog */
						UpdateJTextAreaToFlag(PlayerInformationTextDialog, playerInformationLastName,
							PlayerInformationVerboseCheckBox.isSelected(), playerInformationCurrentFlag);
					}
				}
			}
		});
		PlayerInformationRefreshButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				/* Refresh list of players */
				UpdateJListToSearchString(PlayerInformationPlayerList,
					PlayerInformationSearchTextField.getText());
				/* Refresh player information currently in dialog */
				UpdateJTextAreaToFlag(PlayerInformationTextDialog, playerInformationLastName,
					PlayerInformationVerboseCheckBox.isSelected(), playerInformationCurrentFlag);
			}
		});
		PlayerInformationPlayerList.addListSelectionListener(new ListSelectionListener() {
			@Override
			public void valueChanged(ListSelectionEvent e) {
				/* If there is a valid item currently selected */
				if (PlayerInformationPlayerList.getSelectedIndex() != -1) {
					String newValue = PlayerInformationPlayerList.getSelectedValue().toString();
					if (newValue != null) {
						playerInformationLastName = newValue;
						/* Update player information currently in dialog */
						UpdateJTextAreaToFlag(PlayerInformationTextDialog, playerInformationLastName,
							PlayerInformationVerboseCheckBox.isSelected(), playerInformationCurrentFlag);
					}
				}
			}
		});
		/* Use Box Layout for this tab */
		tabPlayerInformation.setLayout(new BoxLayout(tabPlayerInformation, BoxLayout.X_AXIS));
		/* Layout settings for the tab */
		int playerInfoControlBarMinWidth = 100;
		int playerInfoControlBarPrefWidth = 250;
		int playerInfoControlBarMaxWidth = 400;
		/* Set sizes for radio buttons */
		PlayerInformationHistoryButton.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, CHECKBOX_HEIGHT));
		PlayerInformationHistoryButton.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, CHECKBOX_HEIGHT));
		PlayerInformationHistoryButton.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, CHECKBOX_HEIGHT));
		PlayerInformationRecordsButton.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, CHECKBOX_HEIGHT));
		PlayerInformationRecordsButton.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, CHECKBOX_HEIGHT));
		PlayerInformationRecordsButton.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, CHECKBOX_HEIGHT));
		PlayerInformationEventsAttendedButton.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, CHECKBOX_HEIGHT));
		PlayerInformationEventsAttendedButton.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, CHECKBOX_HEIGHT));
		PlayerInformationEventsAttendedButton.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, CHECKBOX_HEIGHT));
		PlayerInformationNumOutcomesButton.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, CHECKBOX_HEIGHT));
		PlayerInformationNumOutcomesButton.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, CHECKBOX_HEIGHT));
		PlayerInformationNumOutcomesButton.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, CHECKBOX_HEIGHT));
		/* Set sizes for the rest of the control bar */
		PlayerInformationRefreshButton.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, TEXTFIELD_HEIGHT));
		PlayerInformationRefreshButton.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PlayerInformationRefreshButton.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PlayerInformationVerboseCheckBox.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, TEXTFIELD_HEIGHT));
		PlayerInformationVerboseCheckBox.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PlayerInformationVerboseCheckBox.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PlayerInformationSearchTextField.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, TEXTFIELD_HEIGHT));
		PlayerInformationSearchTextField.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PlayerInformationSearchTextField.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PlayerInformationSearchTextField.setToolTipText("Start typing a name to filter results");
		PlayerInformationPlayerListScroll.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, 200));
		PlayerInformationPlayerListScroll.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, Short.MAX_VALUE));
		PlayerInformationPlayerListScroll.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, Short.MAX_VALUE));
		/* Correct Alignments of components in the control bar section */
		PlayerInformationRefreshButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		PlayerInformationVerboseCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		PlayerInformationSearchTextField.setAlignmentX(Component.LEFT_ALIGNMENT);
		PlayerInformationPlayerListScroll.setAlignmentX(Component.LEFT_ALIGNMENT);
		/* Add the radio buttons to the control bar */
		PlayerInformationControlBar.add(PlayerInformationHistoryButton);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PlayerInformationControlBar.add(PlayerInformationRecordsButton);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PlayerInformationControlBar.add(PlayerInformationEventsAttendedButton);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PlayerInformationControlBar.add(PlayerInformationNumOutcomesButton);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		/* Add the rest of the elements in the control bar */
		PlayerInformationControlBar.add(PlayerInformationRefreshButton);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PlayerInformationControlBar.add(PlayerInformationVerboseCheckBox);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PlayerInformationControlBar.add(PlayerInformationSearchTextField);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PlayerInformationControlBar.add(PlayerInformationPlayerListScroll);
		/* Add all the elements to the tab (with spacing) */
		tabPlayerInformation.setBorder(new EmptyBorder(ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING));
		tabPlayerInformation.add(PlayerInformationTextDialogScroll);
		tabPlayerInformation.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		tabPlayerInformation.add(PlayerInformationControlBar);
		/* Configure data in components on Player History tab */
		UpdateJListToFilesInDir(PlayerInformationPlayerList, prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT));

		/* Configure Run Brackets Tab */
		JPanel RunBracketsControlBar = new JPanel();
		RunBracketsControlBar.setLayout(new BoxLayout(RunBracketsControlBar, BoxLayout.Y_AXIS));
		JPanel RunBracketsTextComponents = new JPanel();
		RunBracketsTextComponents.setLayout(new BoxLayout(RunBracketsTextComponents, BoxLayout.Y_AXIS));
		JAliasedCheckBox RunBracketsKeepDataBracketsCheckBox = new JAliasedCheckBox("Keep existing data");
		JAliasedCheckBox RunBracketsKeepDataSeasonsCheckBox = new JAliasedCheckBox("Keep existing data (Season)");
		JAliasedButton RunBracketsAddButton = new JAliasedButton("Add Bracket...");
		JAliasedButton RunBracketsOpenButton = new JAliasedButton("Open...");
		JAliasedButton RunBracketsResetButton = new JAliasedButton("Reset");
		JAliasedButton RunBracketsResetLogButton = new JAliasedButton("Reset Log");
		JAliasedButton RunBracketsRunBracketButton = new JAliasedButton("Run Bracket...");
		JAliasedButton RunBracketsRunSeasonButton = new JAliasedButton("Run Season...");
		JAliasedButton RunBracketsSaveButton = new JAliasedButton("Save As...");
		JAliasedTextArea RunBracketsTextDialog = new JAliasedTextArea();
		JScrollPane RunBracketsTextDialogScroll = new JScrollPane(RunBracketsTextDialog);
		JAliasedTextArea RunBracketsLogDialog = new JAliasedTextArea();
		JScrollPane RunBracketsLogDialogScroll = new JScrollPane(RunBracketsLogDialog);

		RunBracketsKeepDataBracketsCheckBox.setSelected(true);
		RunBracketsKeepDataSeasonsCheckBox.setSelected(false);
		RunBracketsTextDialog.setFont(new Font("monospaced", Font.PLAIN, 12));
		RunBracketsLogDialog.setEditable(false);
		RunBracketsLogDialog.setEnabled(false);

		RunBracketsAddButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Add bracket file", FileDialog.LOAD);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && DestinationFile != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						if (RunBracketsTextDialog.getText().equals("")) {
							RunBracketsTextDialog.setText(RunBracketsTextDialog.getText() +
								FB.getDirectory() + FB.getFile());
						} else {
							RunBracketsTextDialog.setText(RunBracketsTextDialog.getText() +
								"\n" + FB.getDirectory() + FB.getFile());
						}
					} catch (Exception e2) {
						e2.printStackTrace();
					}
				}
			}
		});
		RunBracketsOpenButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Open Season File...", FileDialog.LOAD);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && DestinationFile != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						BufferedReader reader = Files.newBufferedReader(Paths.get(DestinationFile.getAbsolutePath()));
						boolean not_first = false;
						RunBracketsTextDialog.setText("");
						String line;
						while ((line = reader.readLine()) != null) {
							if (not_first) RunBracketsTextDialog.setText(RunBracketsTextDialog.getText() + "\n" + line);
							else RunBracketsTextDialog.setText(RunBracketsTextDialog.getText() + line);
							not_first = true;
						}
						reader.close();
						System.out.println("Read season file");
					} catch (Exception e3) {
						e3.printStackTrace();
					}
				}
			}
		});
		RunBracketsRunBracketButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Run Bracket File...", FileDialog.LOAD);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && DestinationFile != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						String bracket = DestinationFile.getAbsolutePath();
						String k_flag = "";
						if (RunBracketsKeepDataBracketsCheckBox.isSelected()) k_flag = "k";
						int ret = 0;
						ret = DisplayCommandResultsInJTextArea(
								"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
								" -" + k_flag + "b " + bracket, RunBracketsLogDialog, true);
						if (ret != 0) {
							System.err.println("An error occurred running \"" +
								"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
								" -" + k_flag + "b " + bracket + "\"");
						}
					} catch (Exception e3) {
						e3.printStackTrace();
					}
				}
			}
		});
		RunBracketsRunSeasonButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Run Season File...", FileDialog.LOAD);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && DestinationFile != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						String bracket = DestinationFile.getAbsolutePath();
						String k_flag = "";
						if (RunBracketsKeepDataSeasonsCheckBox.isSelected()) k_flag = "k";
						int ret = 0;
						ret = DisplayCommandResultsInJTextArea(
								"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
								" -" + k_flag + "B " + bracket, RunBracketsLogDialog, true);
						if (ret != 0) {
							System.err.println("An error occurred running \"" +
								"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
								" -" + k_flag + "B " + bracket + "\"");
						}
					} catch (Exception e3) {
						e3.printStackTrace();
					}
				}
			}
		});
		RunBracketsResetButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				RunBracketsTextDialog.setText("");
			}
		});
		RunBracketsResetLogButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				RunBracketsLogDialog.setText("");
			}
		});
		RunBracketsSaveButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Save Season As...", FileDialog.SAVE);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && DestinationFile != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						BufferedWriter writer = Files.newBufferedWriter(Paths.get(DestinationFile.getAbsolutePath()));
						writer.write(RunBracketsTextDialog.getText() + "\n");
						writer.close();
						System.out.println("Wrote season to file");
					} catch (Exception e6) {
						e6.printStackTrace();
					}
				}
			}
		});
		/* Use Box Layout for this tab */
		tabRunBrackets.setLayout(new BoxLayout(tabRunBrackets, BoxLayout.X_AXIS));
		/* Layout settings for the tab */
		short runBracketsControlBarMinWidth = 90;
		short runBracketsControlBarPrefWidth = 240;
		short runBracketsControlBarMaxWidth = 360;
		short runBracketsTextComponentsMinWidth = 160;
		short runBracketsTextComponentsPrefWidth = 800;
		short runBracketsTextComponentsMaxWidth = Short.MAX_VALUE;
		/* Set sizes of Text Component section */
		RunBracketsTextDialogScroll.setMinimumSize(new Dimension(runBracketsTextComponentsMinWidth, 100));
		RunBracketsTextDialogScroll.setPreferredSize(new Dimension(runBracketsTextComponentsPrefWidth, 500));
		RunBracketsTextDialogScroll.setMaximumSize(new Dimension(runBracketsTextComponentsMaxWidth, Short.MAX_VALUE));
		RunBracketsLogDialogScroll.setMinimumSize(new Dimension(runBracketsTextComponentsMinWidth, 60));
		RunBracketsLogDialogScroll.setPreferredSize(new Dimension(runBracketsTextComponentsPrefWidth, 140));
		RunBracketsLogDialogScroll.setMaximumSize(new Dimension(runBracketsTextComponentsMaxWidth, 200));
		RunBracketsTextComponents.setMinimumSize(new Dimension(runBracketsTextComponentsMinWidth, 160));
		RunBracketsTextComponents.setPreferredSize(new Dimension(runBracketsTextComponentsPrefWidth, 640));
		RunBracketsTextComponents.setMaximumSize(new Dimension(runBracketsTextComponentsMaxWidth, Short.MAX_VALUE));
		/* Set sizes of Control Bar section */
		RunBracketsKeepDataBracketsCheckBox.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, CHECKBOX_HEIGHT));
		RunBracketsKeepDataBracketsCheckBox.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, CHECKBOX_HEIGHT));
		RunBracketsKeepDataBracketsCheckBox.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, CHECKBOX_HEIGHT));
		RunBracketsKeepDataSeasonsCheckBox.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, CHECKBOX_HEIGHT));
		RunBracketsKeepDataSeasonsCheckBox.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, CHECKBOX_HEIGHT));
		RunBracketsKeepDataSeasonsCheckBox.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, CHECKBOX_HEIGHT));
		RunBracketsOpenButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsOpenButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsOpenButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		RunBracketsSaveButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsSaveButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsSaveButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		RunBracketsRunBracketButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsRunBracketButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsRunBracketButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		RunBracketsRunSeasonButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsRunSeasonButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsRunSeasonButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		RunBracketsAddButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsAddButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsAddButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		RunBracketsResetButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsResetButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsResetButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		RunBracketsResetLogButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsResetLogButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsResetLogButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		/* Correct Alignments of components in the Text Component section */
		RunBracketsTextDialogScroll.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsLogDialogScroll.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsTextComponents.setAlignmentY(Component.TOP_ALIGNMENT);
		RunBracketsTextComponents.setAlignmentX(Component.LEFT_ALIGNMENT);
		/* Correct Alignments of components in the control bar section */
		RunBracketsControlBar.setAlignmentY(Component.TOP_ALIGNMENT);
		RunBracketsControlBar.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsKeepDataBracketsCheckBox.setAlignmentY(Component.TOP_ALIGNMENT);
		RunBracketsKeepDataBracketsCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsKeepDataSeasonsCheckBox.setAlignmentY(Component.TOP_ALIGNMENT);
		RunBracketsKeepDataSeasonsCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsOpenButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsSaveButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsRunBracketButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsRunSeasonButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsAddButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsResetButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsResetLogButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		/* Add all elements in the text component section to the text component panel */
		RunBracketsTextComponents.add(RunBracketsTextDialogScroll);
		RunBracketsTextComponents.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsTextComponents.add(RunBracketsLogDialogScroll);
		/* Add all elements in the control bar to the control bar panel */
		RunBracketsControlBar.add(RunBracketsOpenButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsSaveButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, 4 * ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsKeepDataBracketsCheckBox);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsRunBracketButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsKeepDataSeasonsCheckBox);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsRunSeasonButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, 4 * ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsAddButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsResetButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsResetLogButton);
		/* Add all the elements to the tab (with spacing) */
		tabRunBrackets.setBorder(new EmptyBorder(ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING));
		tabRunBrackets.add(RunBracketsTextComponents);
		tabRunBrackets.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		tabRunBrackets.add(RunBracketsControlBar);

		JFrame frame = new JFrame("graphicalG2ME");
		frame.setDefaultCloseOperation(EXIT_ON_CLOSE);
		frame.setSize(800, 800);
		// TODO: change to center on screen
		frame.setLocation(10, 40);
		frame.setVisible(true);
		// TODO: make icon image
		// frame.setIconImage(new ImageIcon(imgURL).getImage());

		/* Antialiased font tabbed pane */
		JTabbedPane tabbedPane = new JTabbedPane() {
					@Override
					public void paintComponent(Graphics g) {
						Graphics2D graphics2d = (Graphics2D) g;
						graphics2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
						RenderingHints.VALUE_ANTIALIAS_ON);
						super.paintComponent(g);
					}
		};
		tabbedPane.addTab("Settings", tabSettings);
		tabbedPane.addTab("Power Rankings", tabPowerRankings);
		tabbedPane.addTab("Player Info", tabPlayerInformation);
		tabbedPane.addTab("Run Brackets", tabRunBrackets);
		frame.getContentPane().add(tabbedPane, BorderLayout.CENTER);
	}
}
