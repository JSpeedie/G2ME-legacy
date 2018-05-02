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

	/* Prefs Keys */
	private static final String G2ME_DIR="G2ME_DIR";
	private static final String G2ME_PLAYER_DIR="G2ME_PLAYER_DIR";
	private static final String WEIGHT="WEIGHT";
	private static final String USE_GAMES="USE_GAMES";
	private static final String RD_ADJUST_ABSENT="RD_ADJUST_ABSENT";
	private static final String POWER_RANKINGS_VERBOSE="POWER_RANKINGS_VERBOSE";
	private static final String PLAYER_INFO_VERBOSE="PLAYER_INFO_VERBOSE";
	private static final String PLAYER_INFO_RB_SELECTED="PLAYER_INFO_RB_SELECTED";
	/* Prefs defaults */
	private static String G2ME_DIR_DEFAULT="/home/me/G2MEGit/";
	private static String G2ME_PLAYER_DIR_DEFAULT="/home/me/G2MEGit/.players/";
	private static double WEIGHT_DEFAULT=1.0;
	private static boolean USE_GAMES_DEFAULT=false;
	private static boolean RD_ADJUST_ABSENT_DEFAULT=true;
	private static boolean POWER_RANKINGS_VERBOSE_DEFAULT=false;
	private static boolean PLAYER_INFO_VERBOSE_DEFAULT=false;
	private static int PLAYER_INFO_RB_SELECTED_DEFAULT=0;

	private final int ELEMENT_SPACING = 5;
	private final int TEXTFIELD_HEIGHT = 32;
	private final int CHECKBOX_HEIGHT = 24;
	private String playerInformationCurrentFlag = "h";
	private String playerInformationLastName = "";
	private int playerInformationSearchLastLength = 0;
	private String playerRecordsLastName = "";
	private int playerRecordsSearchLastLength = 0;
	private String runBracketsLastFile = "";

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

	public class JAliasedSpinner extends JSpinner {
		public JAliasedSpinner(SpinnerNumberModel s) {
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

	private int DisplayCommandResultsInJTextArea(String command, JTextArea t, boolean stack) {
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

			return pr.waitFor();

		} catch(Exception e) {
		    System.out.println(e.toString());
		    e.printStackTrace();
			return -1;
		}
	}

	private void UpdateJListToFilesInDir(JList l, String PlayerDirPath) {
		File PlayerDirectory = new File(PlayerDirPath);
		if (PlayerDirectory.isDirectory()) {
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

	private void UpdateJTextAreaToFlag(JTextArea t, String playerName, boolean verbose, String flag) {
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

	private void UpdateJTextAreaToFlagWithFilters(JTextArea t, String playerName, boolean verbose,
		String flag, int minEvents, String filterFilePath) {

		Preferences prefs = Preferences.userRoot().node(this.getClass().getName());
		String flags = "-n" + flag;
		String minEventsFlagAndArg = " -m " + minEvents;
		String filter_file_flags_and_arg = "";
		int ret = 0;

		if (!filterFilePath.equals("")) {
			filter_file_flags_and_arg = " -p " + filterFilePath;
		}

		if (verbose) flags = " -nv" + flag;

		ret = DisplayCommandResultsInJTextArea(
			"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
			minEventsFlagAndArg + filter_file_flags_and_arg +
			flags + " " + playerName, t, false);

		if (ret != 0) {
			System.err.println("An error occurred running \"" +
				"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
				minEventsFlagAndArg + filter_file_flags_and_arg +
				flags + " " + playerName + "\"");
		}
	}

	private void UpdateJListToSearchString(JList l, String s) {
		Preferences prefs = Preferences.userRoot().node(this.getClass().getName());
		File PlayerDirectory = new File(prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT));
		File[] listOfFiles = PlayerDirectory.listFiles();
		if (listOfFiles != null) {
			Arrays.sort(listOfFiles);
			ArrayList<String> items = new ArrayList<>();
			for (int i = 0; i < listOfFiles.length; i++) {
				boolean mismatch = false;
				// This file can only match the search if its length is shorter
				// than or equal to the search
				if (s.length() <= listOfFiles[i].getName().length()) {
					for (int j = 0; j < s.length(); j++) {
						if (listOfFiles[i].getName().charAt(j) != s.charAt(j)) {
							mismatch = true;
							break;
						}
					}
					if (mismatch == false) items.add(listOfFiles[i].getName());
				}
			}
			l.setListData(items.toArray());
		}
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
		JPanel SettingsG2MEDirComponents = new JPanel(null);
		SettingsG2MEDirComponents.setLayout(new BoxLayout(SettingsG2MEDirComponents, BoxLayout.X_AXIS));
		JAliasedTextField SettingsG2MEDirTextField = new JAliasedTextField();
		JAliasedButton SettingsG2MEDirBrowseButton = new JAliasedButton("Browse...");
		SettingsG2MEDirBrowseButton.setEnabled(false);
		JPanel SettingsG2MEPlayerDirComponents = new JPanel(null);
		SettingsG2MEPlayerDirComponents.setLayout(new BoxLayout(SettingsG2MEPlayerDirComponents, BoxLayout.X_AXIS));
		JLabel SettingsG2MEPlayerDirLabel = new JLabel("G2ME Players-Directory file path");
		JAliasedTextField SettingsG2MEPlayerDirTextField = new JAliasedTextField();
		JAliasedButton SettingsG2MEPlayerDirBrowseButton = new JAliasedButton("Browse...");
		SettingsG2MEPlayerDirBrowseButton.setEnabled(false);
		JAliasedButton SettingsSaveButton = new JAliasedButton("Save");

		SettingsG2MEDirBrowseButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Select G2ME Directory...", FileDialog.LOAD);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						SettingsG2MEDirTextField.setText(DestinationFile.getAbsolutePath());
					} catch (Exception e1) {
						e1.printStackTrace();
					}
				}
			}
		});

		SettingsG2MEPlayerDirBrowseButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Select G2ME Player Directory...", FileDialog.LOAD);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						SettingsG2MEPlayerDirTextField.setText(DestinationFile.getAbsolutePath());
					} catch (Exception e1) {
						e1.printStackTrace();
					}
				}
			}
		});

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
		SettingsG2MEDirBrowseButton.setMinimumSize(new Dimension(60, TEXTFIELD_HEIGHT));
		SettingsG2MEDirBrowseButton.setPreferredSize(new Dimension(100, TEXTFIELD_HEIGHT));
		SettingsG2MEDirBrowseButton.setMaximumSize(new Dimension(140, TEXTFIELD_HEIGHT));
		SettingsG2MEPlayerDirTextField.setMinimumSize(new Dimension(60, TEXTFIELD_HEIGHT));
		SettingsG2MEPlayerDirTextField.setPreferredSize(new Dimension(60, TEXTFIELD_HEIGHT));
		SettingsG2MEPlayerDirTextField.setMaximumSize(new Dimension(Short.MAX_VALUE, TEXTFIELD_HEIGHT));
		SettingsG2MEPlayerDirBrowseButton.setMinimumSize(new Dimension(60, TEXTFIELD_HEIGHT));
		SettingsG2MEPlayerDirBrowseButton.setPreferredSize(new Dimension(100, TEXTFIELD_HEIGHT));
		SettingsG2MEPlayerDirBrowseButton.setMaximumSize(new Dimension(140, TEXTFIELD_HEIGHT));
		SettingsSaveButton.setMinimumSize(new Dimension(50, TEXTFIELD_HEIGHT));
		SettingsSaveButton.setPreferredSize(new Dimension(70, TEXTFIELD_HEIGHT));
		SettingsSaveButton.setMaximumSize(new Dimension(90, TEXTFIELD_HEIGHT));
		SettingsG2MEDirComponents.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsG2MEPlayerDirComponents.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsG2MEDirLabel.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsG2MEDirTextField.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsG2MEPlayerDirLabel.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsG2MEPlayerDirTextField.setAlignmentX(Component.LEFT_ALIGNMENT);
		SettingsSaveButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		/* Add elements to their "hboxes" */
		SettingsG2MEDirComponents.add(SettingsG2MEDirTextField);
		SettingsG2MEDirComponents.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		SettingsG2MEDirComponents.add(SettingsG2MEDirBrowseButton);
		SettingsG2MEPlayerDirComponents.add(SettingsG2MEPlayerDirTextField);
		SettingsG2MEPlayerDirComponents.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		SettingsG2MEPlayerDirComponents.add(SettingsG2MEPlayerDirBrowseButton);
		/* Add all the elements to the tab (with spacing) */
		tabSettings.setBorder(new EmptyBorder(ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING));
		tabSettings.add(SettingsG2MEDirLabel);
		tabSettings.add(Box.createRigidArea(new Dimension(0,ELEMENT_SPACING)));
		tabSettings.add(SettingsG2MEDirComponents);
		tabSettings.add(Box.createRigidArea(new Dimension(0,ELEMENT_SPACING)));
		tabSettings.add(SettingsG2MEPlayerDirLabel);
		tabSettings.add(Box.createRigidArea(new Dimension(0,ELEMENT_SPACING)));
		tabSettings.add(SettingsG2MEPlayerDirComponents);
		tabSettings.add(Box.createRigidArea(new Dimension(0,ELEMENT_SPACING)));
		tabSettings.add(SettingsSaveButton);

		/* Configure Power Rankings Tab */
		JPanel PowerRankingsControlBar = new JPanel();
		PowerRankingsControlBar.setLayout(new BoxLayout(PowerRankingsControlBar, BoxLayout.Y_AXIS));
		JAliasedButton PowerRankingsGenPRButton = new JAliasedButton("Generate Power Rankings");
		JAliasedCheckBox PowerRankingsVerboseCheckBox = new JAliasedCheckBox("Verbose");
		PowerRankingsVerboseCheckBox.setSelected(prefs.getBoolean(POWER_RANKINGS_VERBOSE, POWER_RANKINGS_VERBOSE_DEFAULT));
		JAliasedTextField PowerRankingsFilterFileTextField = new JAliasedTextField();
		JAliasedButton PowerRankingsFilterFileBrowseButton = new JAliasedButton("Browse For Filter File...");
		JAliasedButton PowerRankingsSaveButton = new JAliasedButton("Save As...");
		JLabel PowerRankingsMinEventsLabel = new JLabel("Min. Events:");
		JPanel PowerRankingsMinEventComponents = new JPanel();
		PowerRankingsMinEventComponents.setLayout(new BoxLayout(PowerRankingsMinEventComponents, BoxLayout.X_AXIS));
		JAliasedSpinner PowerRankingsMinEventsSpinner =
			new JAliasedSpinner(new SpinnerNumberModel(1, 1, 1000, 1));
		PowerRankingsMinEventsSpinner.setToolTipText("Filter Power Ranking Output to Only Include Players Who Have Attended This Many Events");
		JAliasedTextArea PowerRankingsTextDialog = new JAliasedTextArea();
		JScrollPane PowerRankingsTextDialogScroll = new JScrollPane(PowerRankingsTextDialog);

		PowerRankingsTextDialog.setFont(new Font("monospaced", Font.PLAIN, 12));

		PowerRankingsFilterFileBrowseButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Select Filter File...", FileDialog.LOAD);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						PowerRankingsFilterFileTextField.setText(DestinationFile.getAbsolutePath());
					} catch (Exception e1) {
						e1.printStackTrace();
					}
				}
			}
		});

		PowerRankingsGenPRButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				int ret = 0;
				String no_req_flags = "";
				String filter_file_flags_and_arg = "";
				if (PowerRankingsVerboseCheckBox.isSelected()) no_req_flags += "v";
				if (!PowerRankingsFilterFileTextField.getText().equals("")) {
					filter_file_flags_and_arg = " -p " + PowerRankingsFilterFileTextField.getText();
				}

				ret = DisplayCommandResultsInJTextArea(
					"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
					filter_file_flags_and_arg +
					" -m " + PowerRankingsMinEventsSpinner.getValue() +
					" -" + no_req_flags + "O", PowerRankingsTextDialog, false);
				if (ret != 0) {
					System.err.println("An error occurred running \"" +
						"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
						filter_file_flags_and_arg +
						" -m " + PowerRankingsMinEventsSpinner.getValue() +
						" -" + no_req_flags + "O" + "\"");
				}
			}
		});
		PowerRankingsVerboseCheckBox.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				prefs.putBoolean(POWER_RANKINGS_VERBOSE, PowerRankingsVerboseCheckBox.isSelected());
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
				if (FB.getFile() != null && !DestinationFile.isDirectory()) {
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
		tabPowerRankings.setLayout(new BoxLayout(tabPowerRankings, BoxLayout.X_AXIS));
		/* Alignment settings for the tab */
		PowerRankingsTextDialogScroll.setAlignmentY(Component.TOP_ALIGNMENT);
		PowerRankingsControlBar.setAlignmentX(Component.LEFT_ALIGNMENT);
		PowerRankingsControlBar.setAlignmentY(Component.TOP_ALIGNMENT);
		PowerRankingsGenPRButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		PowerRankingsFilterFileTextField.setAlignmentX(Component.LEFT_ALIGNMENT);
		PowerRankingsMinEventComponents.setAlignmentX(Component.LEFT_ALIGNMENT);
		PowerRankingsMinEventsLabel.setAlignmentX(Component.LEFT_ALIGNMENT);
		PowerRankingsMinEventsSpinner.setAlignmentX(Component.LEFT_ALIGNMENT);
		PowerRankingsSaveButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		/* Layout settings for the tab */
		int genPRControlBarMinWidth = 140;
		int genPRControlBarPrefWidth = 300;
		int genPRControlBarMaxWidth = 440;
		/* Dimension settings for the tab */
		PowerRankingsGenPRButton.setMinimumSize(new Dimension(genPRControlBarMinWidth, TEXTFIELD_HEIGHT));
		PowerRankingsGenPRButton.setPreferredSize(new Dimension(genPRControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PowerRankingsGenPRButton.setMaximumSize(new Dimension(genPRControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PowerRankingsGenPRButton.setToolTipText("Generate Power Rankings");
		PowerRankingsVerboseCheckBox.setMinimumSize(new Dimension(genPRControlBarMinWidth, CHECKBOX_HEIGHT));
		PowerRankingsVerboseCheckBox.setPreferredSize(new Dimension(genPRControlBarPrefWidth, CHECKBOX_HEIGHT));
		PowerRankingsVerboseCheckBox.setMaximumSize(new Dimension(genPRControlBarMaxWidth, CHECKBOX_HEIGHT));
		PowerRankingsFilterFileTextField.setMinimumSize(new Dimension(genPRControlBarMinWidth, TEXTFIELD_HEIGHT));
		PowerRankingsFilterFileTextField.setPreferredSize(new Dimension(genPRControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PowerRankingsFilterFileTextField.setMaximumSize(new Dimension(genPRControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PowerRankingsFilterFileBrowseButton.setMinimumSize(new Dimension(genPRControlBarMinWidth, TEXTFIELD_HEIGHT));
		PowerRankingsFilterFileBrowseButton.setPreferredSize(new Dimension(genPRControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PowerRankingsFilterFileBrowseButton.setMaximumSize(new Dimension(genPRControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PowerRankingsFilterFileTextField.setToolTipText("File path for a filter file");
		PowerRankingsMinEventsLabel.setMinimumSize(new Dimension(genPRControlBarMinWidth/2, CHECKBOX_HEIGHT));
		PowerRankingsMinEventsLabel.setPreferredSize(new Dimension(genPRControlBarPrefWidth/2, CHECKBOX_HEIGHT));
		PowerRankingsMinEventsLabel.setMaximumSize(new Dimension(genPRControlBarMaxWidth/2, CHECKBOX_HEIGHT));
		PowerRankingsMinEventsSpinner.setMinimumSize(new Dimension(genPRControlBarMinWidth/2, TEXTFIELD_HEIGHT));
		PowerRankingsMinEventsSpinner.setPreferredSize(new Dimension(genPRControlBarPrefWidth/2, TEXTFIELD_HEIGHT));
		PowerRankingsMinEventsSpinner.setMaximumSize(new Dimension(genPRControlBarMaxWidth/2, TEXTFIELD_HEIGHT));
		JSeparator PowerRankingsSaveBreak = new JSeparator(SwingConstants.HORIZONTAL);
		PowerRankingsSaveBreak.setMinimumSize(new Dimension(genPRControlBarMinWidth - 2 * ELEMENT_SPACING, 3));
		PowerRankingsSaveBreak.setPreferredSize(new Dimension(genPRControlBarPrefWidth - 2 * ELEMENT_SPACING, 3));
		PowerRankingsSaveBreak.setMaximumSize(new Dimension(genPRControlBarMaxWidth - 2 * ELEMENT_SPACING, 3));
		PowerRankingsSaveBreak.setAlignmentX(Component.LEFT_ALIGNMENT);
		PowerRankingsSaveBreak.setAlignmentY(Component.CENTER_ALIGNMENT);
		PowerRankingsSaveButton.setMinimumSize(new Dimension(genPRControlBarMinWidth, TEXTFIELD_HEIGHT));
		PowerRankingsSaveButton.setPreferredSize(new Dimension(genPRControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PowerRankingsSaveButton.setMaximumSize(new Dimension(genPRControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PowerRankingsSaveButton.setToolTipText("Save As...");
		PowerRankingsTextDialogScroll.setMinimumSize(new Dimension(100, 300));
		PowerRankingsTextDialogScroll.setPreferredSize(new Dimension(120, 500));
		PowerRankingsTextDialogScroll.setMaximumSize(new Dimension(Short.MAX_VALUE, Short.MAX_VALUE));
		/* Add Minimum Events Components */
		PowerRankingsMinEventComponents.add(PowerRankingsMinEventsLabel);
		PowerRankingsMinEventComponents.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		PowerRankingsMinEventComponents.add(PowerRankingsMinEventsSpinner);
		/* Add all elements in the control bar to the control bar panel */
		PowerRankingsControlBar.add(PowerRankingsGenPRButton);
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PowerRankingsControlBar.add(PowerRankingsVerboseCheckBox);
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PowerRankingsControlBar.add(PowerRankingsMinEventComponents);
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PowerRankingsControlBar.add(PowerRankingsFilterFileTextField);
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PowerRankingsControlBar.add(PowerRankingsFilterFileBrowseButton);
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(0, 2 * ELEMENT_SPACING)));
		PowerRankingsControlBar.add(PowerRankingsSaveBreak);
		PowerRankingsControlBar.add(Box.createRigidArea(new Dimension(0, 2 * ELEMENT_SPACING)));
		PowerRankingsControlBar.add(PowerRankingsSaveButton);
		/* Add all the elements to the tab (with spacing) */
		tabPowerRankings.setBorder(new EmptyBorder(ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING, ELEMENT_SPACING));
		tabPowerRankings.add(PowerRankingsTextDialogScroll);
		tabPowerRankings.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		tabPowerRankings.add(PowerRankingsControlBar);

		/* Configure Player Information Tab */
		JPanel PlayerInformationControlBar = new JPanel();
		PlayerInformationControlBar.setLayout(new BoxLayout(PlayerInformationControlBar, BoxLayout.Y_AXIS));
		JAliasedButton PlayerInformationRefreshButton = new JAliasedButton("Refresh");
		JAliasedCheckBox PlayerInformationVerboseCheckBox = new JAliasedCheckBox("Verbose");
		PlayerInformationVerboseCheckBox.setSelected(prefs.getBoolean(PLAYER_INFO_VERBOSE, PLAYER_INFO_VERBOSE_DEFAULT));
		JAliasedTextField PlayerInformationSearchTextField = new JAliasedTextField();
		JAliasedList PlayerInformationPlayerList = new JAliasedList();
		JScrollPane PlayerInformationPlayerListScroll = new JScrollPane(PlayerInformationPlayerList);
		JAliasedTextArea PlayerInformationTextDialog = new JAliasedTextArea();
		JScrollPane PlayerInformationTextDialogScroll = new JScrollPane(PlayerInformationTextDialog);
		JAliasedTextField PlayerInformationFilterFileTextField = new JAliasedTextField();
		JAliasedButton PlayerInformationFilterFileBrowseButton = new JAliasedButton("Browse For Filter File...");
		JLabel PlayerInformationMinEventsLabel = new JLabel("Min. Events:");
		JPanel PlayerInformationMinEventComponents = new JPanel();
		PlayerInformationMinEventComponents.setLayout(new BoxLayout(PlayerInformationMinEventComponents, BoxLayout.X_AXIS));
		JAliasedSpinner PlayerInformationMinEventsSpinner =
			new JAliasedSpinner(new SpinnerNumberModel(1, 1, 1000, 1));
		PlayerInformationMinEventsSpinner.setToolTipText("Filter Power Ranking Output to Only Include Players Who Have Attended This Many Events");

		JAliasedRadioButton PlayerInformationHistoryButton = new JAliasedRadioButton("Outcome History");
		PlayerInformationHistoryButton.setToolTipText("Opponent, Date, Tournament and Glicko2 Data After Every Outcome (Set/Game)");
		JAliasedRadioButton PlayerInformationRecordsButton = new JAliasedRadioButton("Records/Head-to-Heads");
		PlayerInformationRecordsButton.setToolTipText("Wins, Ties, Losses Against All Players Played");
		JAliasedRadioButton PlayerInformationEventsAttendedButton = new JAliasedRadioButton("Events Attended");
		PlayerInformationEventsAttendedButton.setToolTipText("Names of All Events Attended");
		JAliasedRadioButton PlayerInformationNumOutcomesButton = new JAliasedRadioButton("Number of Outcomes (Sets/Games) Played");
		PlayerInformationNumOutcomesButton.setToolTipText("Number of Outcomes (Sets/Games) Played");
		String[] playerInfoFlags = new String[4];
		playerInfoFlags[0] = "h";
		playerInfoFlags[1] = "R";
		playerInfoFlags[2] = "A";
		playerInfoFlags[3] = "c";
		JAliasedRadioButton[] PlayerInfoRadioButtonArray = new JAliasedRadioButton[4];
		PlayerInfoRadioButtonArray[0] = PlayerInformationHistoryButton;
		PlayerInfoRadioButtonArray[1] = PlayerInformationRecordsButton;
		PlayerInfoRadioButtonArray[2] = PlayerInformationEventsAttendedButton;
		PlayerInfoRadioButtonArray[3] = PlayerInformationNumOutcomesButton;

		PlayerInformationHistoryButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				PlayerInformationVerboseCheckBox.setEnabled(true);
				PlayerInformationMinEventsSpinner.setEnabled(false);
				PlayerInformationFilterFileTextField.setEnabled(false);
				PlayerInformationFilterFileBrowseButton.setEnabled(false);
				playerInformationCurrentFlag = playerInfoFlags[0];
				prefs.putInt(PLAYER_INFO_RB_SELECTED, 0);
			}
		});
		PlayerInformationRecordsButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				PlayerInformationVerboseCheckBox.setEnabled(false);
				PlayerInformationMinEventsSpinner.setEnabled(true);
				PlayerInformationFilterFileTextField.setEnabled(true);
				PlayerInformationFilterFileBrowseButton.setEnabled(true);
				playerInformationCurrentFlag = playerInfoFlags[1];
				prefs.putInt(PLAYER_INFO_RB_SELECTED, 1);
			}
		});
		PlayerInformationEventsAttendedButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				PlayerInformationVerboseCheckBox.setEnabled(false);
				PlayerInformationMinEventsSpinner.setEnabled(false);
				PlayerInformationFilterFileTextField.setEnabled(false);
				PlayerInformationFilterFileBrowseButton.setEnabled(false);
				playerInformationCurrentFlag = playerInfoFlags[2];
				prefs.putInt(PLAYER_INFO_RB_SELECTED, 2);
			}
		});
		PlayerInformationNumOutcomesButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				PlayerInformationVerboseCheckBox.setEnabled(false);
				PlayerInformationMinEventsSpinner.setEnabled(false);
				PlayerInformationFilterFileTextField.setEnabled(false);
				PlayerInformationFilterFileBrowseButton.setEnabled(false);
				playerInformationCurrentFlag = playerInfoFlags[3];
				prefs.putInt(PLAYER_INFO_RB_SELECTED, 3);
			}
		});
		/* Remember which one was selected, and set GUI accordingly */
		int previousRBSelected = prefs.getInt(PLAYER_INFO_RB_SELECTED, PLAYER_INFO_RB_SELECTED_DEFAULT);
		PlayerInfoRadioButtonArray[previousRBSelected].setSelected(true);
		playerInformationCurrentFlag = playerInfoFlags[previousRBSelected];

		if (previousRBSelected == 0) {
			PlayerInformationVerboseCheckBox.setEnabled(true);
		} else {
			PlayerInformationVerboseCheckBox.setEnabled(false);
		}
		if (previousRBSelected == 1) {
			PlayerInformationMinEventsSpinner.setEnabled(true);
			PlayerInformationFilterFileTextField.setEnabled(true);
			PlayerInformationFilterFileBrowseButton.setEnabled(true);
		} else {
			PlayerInformationMinEventsSpinner.setEnabled(false);
			PlayerInformationFilterFileTextField.setEnabled(false);
			PlayerInformationFilterFileBrowseButton.setEnabled(false);
		}

		ButtonGroup PlayerInformationButtonGroup = new ButtonGroup();
		PlayerInformationButtonGroup.add(PlayerInformationHistoryButton);
		PlayerInformationButtonGroup.add(PlayerInformationRecordsButton);
		PlayerInformationButtonGroup.add(PlayerInformationEventsAttendedButton);
		PlayerInformationButtonGroup.add(PlayerInformationNumOutcomesButton);

		PlayerInformationVerboseCheckBox.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				prefs.putBoolean(PLAYER_INFO_VERBOSE, PlayerInformationVerboseCheckBox.isSelected());
			}
		});

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
		PlayerInformationFilterFileBrowseButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				FileDialog FB = new java.awt.FileDialog((java.awt.Frame) null,
					"Select Filter File...", FileDialog.LOAD);
				FB.setDirectory(prefs.get(G2ME_DIR, G2ME_DIR_DEFAULT));
				FB.setVisible(true);

				/* If a file was successfully chosen */
				File DestinationFile = new File(FB.getDirectory() + FB.getFile());
				if (FB.getFile() != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						PlayerInformationFilterFileTextField.setText(DestinationFile.getAbsolutePath());
					} catch (Exception e1) {
						e1.printStackTrace();
					}
				}
			}
		});
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
						UpdateJTextAreaToFlagWithFilters(PlayerInformationTextDialog, playerInformationLastName,
							PlayerInformationVerboseCheckBox.isSelected(), playerInformationCurrentFlag,
							(int)PlayerInformationMinEventsSpinner.getValue(), PlayerInformationFilterFileTextField.getText());
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
				UpdateJTextAreaToFlagWithFilters(PlayerInformationTextDialog, playerInformationLastName,
					PlayerInformationVerboseCheckBox.isSelected(), playerInformationCurrentFlag,
					(int)PlayerInformationMinEventsSpinner.getValue(), PlayerInformationFilterFileTextField.getText());
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
						UpdateJTextAreaToFlagWithFilters(PlayerInformationTextDialog, playerInformationLastName,
							PlayerInformationVerboseCheckBox.isSelected(), playerInformationCurrentFlag,
							(int)PlayerInformationMinEventsSpinner.getValue(), PlayerInformationFilterFileTextField.getText());
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
		JSeparator PlayerInformationPlayerListBreak = new JSeparator(SwingConstants.HORIZONTAL);
		PlayerInformationFilterFileTextField.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, TEXTFIELD_HEIGHT));
		PlayerInformationFilterFileTextField.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PlayerInformationFilterFileTextField.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PlayerInformationFilterFileBrowseButton.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, TEXTFIELD_HEIGHT));
		PlayerInformationFilterFileBrowseButton.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PlayerInformationFilterFileBrowseButton.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PlayerInformationFilterFileTextField.setToolTipText("File path for a filter file");
		PlayerInformationMinEventsLabel.setMinimumSize(new Dimension(playerInfoControlBarMinWidth/2, CHECKBOX_HEIGHT));
		PlayerInformationMinEventsLabel.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth/2, CHECKBOX_HEIGHT));
		PlayerInformationMinEventsLabel.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth/2, CHECKBOX_HEIGHT));
		PlayerInformationMinEventsSpinner.setMinimumSize(new Dimension(playerInfoControlBarMinWidth/2, TEXTFIELD_HEIGHT));
		PlayerInformationMinEventsSpinner.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth/2, TEXTFIELD_HEIGHT));
		PlayerInformationMinEventsSpinner.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth/2, TEXTFIELD_HEIGHT));
		PlayerInformationPlayerListBreak.setMinimumSize(new Dimension(playerInfoControlBarMinWidth - 2 * ELEMENT_SPACING, 3));
		PlayerInformationPlayerListBreak.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth - 2 * ELEMENT_SPACING, 3));
		PlayerInformationPlayerListBreak.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth - 2 * ELEMENT_SPACING, 3));
		PlayerInformationSearchTextField.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, TEXTFIELD_HEIGHT));
		PlayerInformationSearchTextField.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, TEXTFIELD_HEIGHT));
		PlayerInformationSearchTextField.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, TEXTFIELD_HEIGHT));
		PlayerInformationSearchTextField.setToolTipText("Start typing a name to filter results");
		PlayerInformationPlayerListScroll.setMinimumSize(new Dimension(playerInfoControlBarMinWidth, 200));
		PlayerInformationPlayerListScroll.setPreferredSize(new Dimension(playerInfoControlBarPrefWidth, Short.MAX_VALUE));
		PlayerInformationPlayerListScroll.setMaximumSize(new Dimension(playerInfoControlBarMaxWidth, Short.MAX_VALUE));
		/* Add Minimum Events Components */
		PlayerInformationMinEventComponents.add(PlayerInformationMinEventsLabel);
		PlayerInformationMinEventComponents.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		PlayerInformationMinEventComponents.add(PlayerInformationMinEventsSpinner);
		/* Correct Alignments of components in the control bar section */
		PlayerInformationRefreshButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		PlayerInformationVerboseCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		PlayerInformationMinEventComponents.setAlignmentX(Component.LEFT_ALIGNMENT);
		PlayerInformationFilterFileTextField.setAlignmentX(Component.LEFT_ALIGNMENT);
		PlayerInformationFilterFileBrowseButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		PlayerInformationPlayerListBreak.setAlignmentX(Component.LEFT_ALIGNMENT);
		PlayerInformationPlayerListBreak.setAlignmentY(Component.CENTER_ALIGNMENT);
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
		PlayerInformationControlBar.add(PlayerInformationMinEventComponents);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PlayerInformationControlBar.add(PlayerInformationFilterFileTextField);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		PlayerInformationControlBar.add(PlayerInformationFilterFileBrowseButton);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, 2 * ELEMENT_SPACING)));
		PlayerInformationControlBar.add(PlayerInformationPlayerListBreak);
		PlayerInformationControlBar.add(Box.createRigidArea(new Dimension(0, 2 * ELEMENT_SPACING)));
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
		JPanel RunBracketsWeightComponents = new JPanel();
		RunBracketsWeightComponents.setLayout(new BoxLayout(RunBracketsWeightComponents, BoxLayout.X_AXIS));
		JAliasedCheckBox RunBracketsKeepDataBracketsCheckBox = new JAliasedCheckBox("Keep existing data");
		RunBracketsKeepDataBracketsCheckBox.setToolTipText("Run bracket using existing player data");
		JAliasedCheckBox RunBracketsKeepDataSeasonsCheckBox = new JAliasedCheckBox("Keep existing data (Season)");
		RunBracketsKeepDataSeasonsCheckBox.setToolTipText("Run season using existing player data");
		JAliasedCheckBox RunBracketsUseGamesCheckBox = new JAliasedCheckBox("Use Games");
		RunBracketsUseGamesCheckBox.setSelected(prefs.getBoolean(USE_GAMES, USE_GAMES_DEFAULT));
		RunBracketsUseGamesCheckBox.setToolTipText("Use Games as Outcomes Rather Than Sets (Not Recommended)");
		JAliasedCheckBox RunBracketsRDAdjustAbsentCheckBox = new JAliasedCheckBox("RD Adjust Absent Players");
		RunBracketsRDAdjustAbsentCheckBox.setSelected(prefs.getBoolean(RD_ADJUST_ABSENT, RD_ADJUST_ABSENT_DEFAULT));
		RunBracketsRDAdjustAbsentCheckBox.setToolTipText("Adjust the Rating Deviation of Absent Players (Recommended)");
		JAliasedButton RunBracketsAddButton = new JAliasedButton("Add Bracket...");
		JAliasedButton RunBracketsOpenButton = new JAliasedButton("Open...");
		JAliasedButton RunBracketsClearButton = new JAliasedButton("Clear");
		JAliasedButton RunBracketsResetButton = new JAliasedButton("Reset");
		JAliasedButton RunBracketsClearLogButton = new JAliasedButton("Clear Log");
		JAliasedButton RunBracketsRunBracketButton = new JAliasedButton("Run Bracket...");
		JAliasedButton RunBracketsRunSeasonButton = new JAliasedButton("Run Season...");
		JAliasedButton RunBracketsSaveButton = new JAliasedButton("Save As...");
		JLabel RunBracketsWeightLabel = new JLabel("Weight:");
		JAliasedSpinner RunBracketsWeightSpinner =
			new JAliasedSpinner(new SpinnerNumberModel(prefs.getDouble(WEIGHT, WEIGHT_DEFAULT), 0.0, 1000.0, 0.1));
		RunBracketsWeightSpinner.setToolTipText("Weigh Tournament at Given Value (Not Recommended to be anything other than 1)");
		JAliasedTextArea RunBracketsTextDialog = new JAliasedTextArea();
		JScrollPane RunBracketsTextDialogScroll = new JScrollPane(RunBracketsTextDialog);
		JAliasedTextArea RunBracketsLogDialog = new JAliasedTextArea();
		JScrollPane RunBracketsLogDialogScroll = new JScrollPane(RunBracketsLogDialog);

		RunBracketsKeepDataBracketsCheckBox.setSelected(true);
		RunBracketsKeepDataSeasonsCheckBox.setSelected(false);
		RunBracketsWeightSpinner.addChangeListener(new ChangeListener() {
			@Override
			public void stateChanged(ChangeEvent e) {
				prefs.putDouble(WEIGHT, (double)RunBracketsWeightSpinner.getValue());
			}
		});
		RunBracketsUseGamesCheckBox.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				prefs.putBoolean(USE_GAMES, RunBracketsUseGamesCheckBox.isSelected());
			}
		});
		RunBracketsRDAdjustAbsentCheckBox.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				prefs.putBoolean(RD_ADJUST_ABSENT, RunBracketsRDAdjustAbsentCheckBox.isSelected());
			}
		});
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
				if (FB.getFile() != null && !DestinationFile.isDirectory()) {
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
				if (FB.getFile() != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					runBracketsLastFile = DestinationFile.getAbsolutePath();
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
				if (FB.getFile() != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						String bracket = DestinationFile.getAbsolutePath();
						String no_req_flags = "";
						if (!RunBracketsRDAdjustAbsentCheckBox.isSelected()) no_req_flags += "0";
						if (RunBracketsUseGamesCheckBox.isSelected()) no_req_flags += "g";
						if (RunBracketsKeepDataBracketsCheckBox.isSelected()) no_req_flags += "k";
						int ret = 0;
						ret = DisplayCommandResultsInJTextArea(
								"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
								" -w " + RunBracketsWeightSpinner.getValue() +
								" -" + no_req_flags + "b " + bracket, RunBracketsLogDialog, true);
						if (ret != 0) {
							System.err.println("An error occurred running \"" +
								"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
								" -w " + RunBracketsWeightSpinner.getValue() +
								" -" + no_req_flags + "b " + bracket + "\"");
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
				if (FB.getFile() != null && !DestinationFile.isDirectory()) {
					System.out.println("File path chosen = " + DestinationFile.getAbsolutePath());
					try {
						String bracket = DestinationFile.getAbsolutePath();
						String no_req_flags = "";
						if (!RunBracketsRDAdjustAbsentCheckBox.isSelected()) no_req_flags += "0";
						if (RunBracketsUseGamesCheckBox.isSelected()) no_req_flags += "g";
						if (RunBracketsKeepDataSeasonsCheckBox.isSelected()) no_req_flags += "k";
						int ret = 0;
						ret = DisplayCommandResultsInJTextArea(
								"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
								" -w " + RunBracketsWeightSpinner.getValue() +
								" -" + no_req_flags + "B " + bracket, RunBracketsLogDialog, true);
						if (ret != 0) {
							System.err.println("An error occurred running \"" +
								"G2ME -d " + prefs.get(G2ME_PLAYER_DIR, G2ME_PLAYER_DIR_DEFAULT) +
								" -w " + RunBracketsWeightSpinner.getValue() +
								" -" + no_req_flags + "B " + bracket + "\"");
						}
					} catch (Exception e3) {
						e3.printStackTrace();
					}
				}
			}
		});
		RunBracketsClearButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				RunBracketsTextDialog.setText("");
			}
		});
		RunBracketsResetButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				try {
					BufferedReader reader = Files.newBufferedReader(Paths.get(runBracketsLastFile));
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
		});
		RunBracketsClearLogButton.addActionListener(new ActionListener() {
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
				if (FB.getFile() != null && !DestinationFile.isDirectory()) {
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
		RunBracketsWeightLabel.setMinimumSize(new Dimension(runBracketsControlBarMinWidth/2, CHECKBOX_HEIGHT));
		RunBracketsWeightLabel.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth/2, CHECKBOX_HEIGHT));
		RunBracketsWeightLabel.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth/2, CHECKBOX_HEIGHT));
		RunBracketsWeightSpinner.setMinimumSize(new Dimension(runBracketsControlBarMinWidth/2, TEXTFIELD_HEIGHT));
		RunBracketsWeightSpinner.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth/2, TEXTFIELD_HEIGHT));
		RunBracketsWeightSpinner.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth/2, TEXTFIELD_HEIGHT));
		RunBracketsUseGamesCheckBox.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, CHECKBOX_HEIGHT));
		RunBracketsUseGamesCheckBox.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, CHECKBOX_HEIGHT));
		RunBracketsUseGamesCheckBox.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, CHECKBOX_HEIGHT));
		RunBracketsRDAdjustAbsentCheckBox.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, CHECKBOX_HEIGHT));
		RunBracketsRDAdjustAbsentCheckBox.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, CHECKBOX_HEIGHT));
		RunBracketsRDAdjustAbsentCheckBox.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, CHECKBOX_HEIGHT));
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
		RunBracketsClearButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsClearButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsClearButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		RunBracketsResetButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsResetButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsResetButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		RunBracketsClearLogButton.setMinimumSize(new Dimension(runBracketsControlBarMinWidth, TEXTFIELD_HEIGHT));
		RunBracketsClearLogButton.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth, TEXTFIELD_HEIGHT));
		RunBracketsClearLogButton.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth, TEXTFIELD_HEIGHT));
		/* Correct Alignments of components in the Text Component section */
		RunBracketsTextDialogScroll.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsLogDialogScroll.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsTextComponents.setAlignmentY(Component.TOP_ALIGNMENT);
		RunBracketsTextComponents.setAlignmentX(Component.LEFT_ALIGNMENT);
		/* Correct Alignments of components in the control bar section */
		RunBracketsControlBar.setAlignmentY(Component.TOP_ALIGNMENT);
		RunBracketsControlBar.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsKeepDataBracketsCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsWeightComponents.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsUseGamesCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsRDAdjustAbsentCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsKeepDataBracketsCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsKeepDataSeasonsCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsOpenButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsSaveButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsRunBracketButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsRunSeasonButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsAddButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsClearButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsResetButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		RunBracketsClearLogButton.setAlignmentX(Component.LEFT_ALIGNMENT);
		/* Add all elements in the text component section to the text component panel */
		RunBracketsTextComponents.add(RunBracketsTextDialogScroll);
		RunBracketsTextComponents.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsTextComponents.add(RunBracketsLogDialogScroll);
		/* Organize the bracket weight elements */
		RunBracketsWeightComponents.add(RunBracketsWeightLabel);
		RunBracketsWeightComponents.add(Box.createRigidArea(new Dimension(ELEMENT_SPACING, 0)));
		RunBracketsWeightComponents.add(RunBracketsWeightSpinner);
		/* Add the rest of the control bar elements */
		JSeparator Break = new JSeparator(SwingConstants.HORIZONTAL);
		Break.setMinimumSize(new Dimension(runBracketsControlBarMinWidth - 2 * ELEMENT_SPACING, 3));
		Break.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth - 2 * ELEMENT_SPACING, 3));
		Break.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth - 2 * ELEMENT_SPACING, 3));
		Break.setAlignmentX(Component.LEFT_ALIGNMENT);
		Break.setAlignmentY(Component.CENTER_ALIGNMENT);
		JSeparator Break2 = new JSeparator(SwingConstants.HORIZONTAL);
		Break2.setMinimumSize(new Dimension(runBracketsControlBarMinWidth - 2 * ELEMENT_SPACING, 3));
		Break2.setPreferredSize(new Dimension(runBracketsControlBarPrefWidth - 2 * ELEMENT_SPACING, 3));
		Break2.setMaximumSize(new Dimension(runBracketsControlBarMaxWidth - 2 * ELEMENT_SPACING, 3));
		Break2.setAlignmentX(Component.LEFT_ALIGNMENT);
		Break2.setAlignmentY(Component.CENTER_ALIGNMENT);
		RunBracketsControlBar.add(RunBracketsOpenButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsSaveButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsAddButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsClearButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsResetButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, 2 * ELEMENT_SPACING)));
		RunBracketsControlBar.add(Break);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, 2 * ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsWeightComponents);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsUseGamesCheckBox);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsRDAdjustAbsentCheckBox);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, 2 * ELEMENT_SPACING)));
		RunBracketsControlBar.add(Break2);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, 2 * ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsKeepDataBracketsCheckBox);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsRunBracketButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsKeepDataSeasonsCheckBox);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsRunSeasonButton);
		RunBracketsControlBar.add(Box.createRigidArea(new Dimension(0, ELEMENT_SPACING)));
		RunBracketsControlBar.add(RunBracketsClearLogButton);
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
