package com.sony.soe.swg.cts.ui;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JFrame;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.WindowConstants;
import javax.swing.tree.DefaultTreeModel;

import com.sony.soe.swg.cts.*;

/**
 * A simple UI to get and display the values associated with a character object.
 * 
 * @author bmauer
 *
 */
public class SWGCharacterTransferUI extends JFrame
                                    implements ActionListener
{
    /**
     * Default way to set the UI to look like the OS it is running on
     */
    static
    {
        try
        {
            UIManager.setLookAndFeel( UIManager.getSystemLookAndFeelClassName() );
        }
        catch ( Exception e )
        {
            JFrame.setDefaultLookAndFeelDecorated(true);
        }
    }
 
    /**
     * For serialized objects.
     */
    public static final long serialVersionUID = 0;
    
    /**
     * Database connection with logic for character object retrieval.
     */
    private DataConnection      _connection;
    
    /**
     * UI Objects to get values from.
     */
    private JPanel              _mainPanel;
    private JComboBox           _databaseComboBox;
    private JComboBox           _serverComboBox;
    private JTextField          _characterIdTextField;
    private JButton             _getInfoButton;
    private JTree               _infoTree;
    private DefaultTreeModel    _model;

    /**
     * Create the datastructures for this UI.
     * @param title
     */
    public SWGCharacterTransferUI( String title )
    {
        super(title);
        _connection = null;
        _model = new DefaultTreeModel( null );
    }
    
    /**
     * Initialize, layout, and display the UI.
     *
     */
    public void init()
    {
        setDefaultCloseOperation( WindowConstants.EXIT_ON_CLOSE );
        setSize( new Dimension( 800, 600 ) );
        
        _mainPanel = new JPanel( new GridBagLayout() );
        
        GridBagConstraints con = new GridBagConstraints( 0,
                                                         0,
                                                         1,
                                                         1,
                                                         0.0,
                                                         0.0,
                                                         GridBagConstraints.NORTHWEST,
                                                         GridBagConstraints.NONE,
                                                         new Insets( 3, 3, 3, 3 ),
                                                         0,
                                                         0 );
        
        // Where to get information
        JPanel wherePanel = new JPanel( new GridBagLayout() );
        JLabel databaseLabel = new JLabel( "Database" );
        wherePanel.add( databaseLabel, con );
        con.gridx = 1;
        con.weightx = 100.0;
        con.fill = GridBagConstraints.HORIZONTAL;
        String databaseArray[] = { "SWODB" };
        _databaseComboBox = new JComboBox( databaseArray );
        wherePanel.add( _databaseComboBox, con );
        con.gridy = 1;
        con.gridx = 0;
        con.weightx = 0.0;
        con.fill = GridBagConstraints.NONE;
        JLabel serverLabel = new JLabel( "Server" );
        wherePanel.add( serverLabel, con );
        con.gridx = 1;
        con.weightx = 100.0;
        con.fill = GridBagConstraints.HORIZONTAL;
        String serverArray[] = { "sdtswg01" };
        _serverComboBox = new JComboBox( serverArray );
        wherePanel.add( _serverComboBox, con );
        
        con.gridx = 0;
        con.gridy = 0;
        _mainPanel.add( wherePanel, con );
        
        // What information to get it base upon
        // This could eventually be:
        //      station id (for multiple characters)
        //      character name
        //      all characters 'LIKE' a string
        // It would use radio buttons to determine which value to use.
        JPanel whatPanel = new JPanel( new GridBagLayout() );
        con.gridy = 0;
        con.gridx = 0;
        con.weightx = 0.0;
        con.fill = GridBagConstraints.NONE;
        JLabel characterIdLabel = new JLabel( "Character Id" );
        whatPanel.add( characterIdLabel, con );
        con.gridx = 1;
        con.weightx = 100.0;
        con.fill = GridBagConstraints.HORIZONTAL;
        _characterIdTextField = new JTextField();
        whatPanel.add( _characterIdTextField, con );
        
        con.gridx = 1;
        con.gridy = 0;
        _mainPanel.add( whatPanel, con );
        
        con.gridx = 0;
        con.gridy = 1;
        con.gridwidth = 2;
        con.weightx = 0.0;
        con.fill = GridBagConstraints.NONE;
        con.anchor = GridBagConstraints.CENTER;
        _getInfoButton = new JButton( "Get Info" );
        _getInfoButton.setActionCommand( "get_info" );
        _getInfoButton.addActionListener( this );
        _mainPanel.add( _getInfoButton, con );
        
        con.gridy = 2;
        con.weightx = 100.0;
        con.weighty = 100.0;
        con.fill = GridBagConstraints.BOTH;
        con.anchor = GridBagConstraints.NORTHWEST;
        _infoTree = new JTree( _model );
        JScrollPane treePane = new JScrollPane( _infoTree );
        _mainPanel.add( treePane, con );

        getContentPane().add( _mainPanel, BorderLayout.CENTER );
        
        pack();
        // pack() has a known bug when used with JTextField (it uses the text in the field to determine minSize).  Use setSize() and invalidate().
        setSize( new Dimension( 744, 439 ) );
        invalidate();
    }
    
    /**
     * Enable or disable a component and all of its children.
     * 
     * @param container Container to disable/enable
     * @param enable If true, enable.  Otherwise disable.
     */
    private void setEnableComponentWithChildren( Container container, boolean enable )
    {
        if ( !isVisible() )
        {
            return;
        }
        container.setEnabled( enable );
        Component children[] = container.getComponents();
        if ( children != null )
        {
            for ( int i = 0; i < children.length; i++ )
            {
                try
                {
                    if ( Class.forName( "java.awt.Container" ).isInstance( children[i] ) )
                    {
                        setEnableComponentWithChildren( (Container)children[i], enable );
                    }
                    else
                    {
                        children[i].setEnabled( enable );
                    }
                }
                catch (Exception e)
                {
                }
            }
        }
    }
    
    /**
     * Handles the logic of what to do based on buttons being pressed.  Currently handles
     * only the Get Info button.
     */
    public void actionPerformed(ActionEvent e)
    {
        if ( "get_info".equals( e.getActionCommand() ) )
        {
            setEnableComponentWithChildren( _mainPanel, false );
            // TODO: See if it's the same DB to save time
            if ( _connection != null )
            {
                _connection.disconnect();
                _connection = null;
            }
            _connection = new DataConnection( _databaseComboBox.getSelectedItem().toString(), _serverComboBox.getSelectedItem().toString(), "changeme" );
            _connection.init();
            long characterId = -1L;
            try
            {
                characterId = Long.parseLong( _characterIdTextField.getText() );
            }
            catch ( NumberFormatException ex )
            {
                // TODO:  Show dialog to user
                ex.printStackTrace();
            }
            
            SWGCharacterObject character = _connection.loadCharacter( characterId );
            _model.setRoot( new CharacterTreeNode( character, null ) );
            setEnableComponentWithChildren( _mainPanel, true );
        }
   }

    /**
     * Starts, inits, an sets the UI to visible.
     * @param args ignored
     */
    public static void main(String[] args)
    {
        SWGCharacterTransferUI page = new SWGCharacterTransferUI( "C4SourceContentsWriter" );
        page.init();
        page.setVisible(true);
    }
}
