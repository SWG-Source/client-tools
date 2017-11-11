package com.sony.soe.swg.cts.ui;

import javax.swing.tree.TreeNode;

import com.sony.soe.swg.cts.*;

/**
 * TreeNode which knows how to display Object and Character data.
 * @author bmauer
 *
 */
public class CharacterTreeNode extends ObjectTreeNode
{
    private SWGCharacterObject  _character;
    
    CharacterTreeNode( SWGCharacterObject character, TreeNode parent )
    {
        super( character, parent );
        _character = character;
        _children.add( new StringTreeNode( "Biography", new String[] { _character.getBiography() }, this ) );
        _children.add( new StringTreeNode( "Houses", _character.getHouses(), this ) );
        _children.add( new StringTreeNode( "Installations", _character.getInstallations(), this ) );
        _children.add( new StringTreeNode( "Harvesters", _character.getHarvesters(), this ) );
    }
}
