package com.sony.soe.swg.cts.ui;

import java.util.Enumeration;
import java.util.Vector;

import javax.swing.tree.TreeNode;

import com.sony.soe.swg.cts.*;

/**
 * TreeNode which knows how to display objvar, object array, and string data.
 * @author bmauer
 *
 */
public class StringTreeNode implements TreeNode
{
    private String           _name;
    private Vector _children;
    private TreeNode         _parent;
    
    StringTreeNode( String name, Object children[], TreeNode parent )
    {
        _name = name;
        _children = new Vector();
        if ( children != null )
        {
            for ( int i = 0; i < children.length; i++ )
            {
                if ( String.class.isInstance( children[i] ) )
                {
                    _children.add( new StringTreeNode( (String)children[i], null, this ) );
                }
                else if ( SWGObject.class.isInstance( children[i] ) )
                {
                    _children.add( new ObjectTreeNode( (SWGObject)children[i], this ) );
                }
                else if ( SWGObjectVariable.class.isInstance( children[i] ) )
                {
                    SWGObjectVariable objvar = (SWGObjectVariable)children[i];
                    _children.add( new StringTreeNode( objvar.getName() + ":  " + objvar.getValue(), null, this ) );
                }
            }
        }
        _parent = parent;
    }

    public TreeNode getChildAt( int index )
    {
        return (TreeNode)_children.get( index );
    }

    public int getChildCount()
    {
        return _children.size();
    }

    public TreeNode getParent()
    {
        return _parent;
    }

    public int getIndex(TreeNode index)
    {
        return 0;
    }

    public boolean getAllowsChildren()
    {
        return true;
    }

    public boolean isLeaf()
    {
        return _children.size() == 0;
    }

    public Enumeration children()
    {
        return _children.elements();
    }
    
    public String toString()
    {
        if ( _name != null )
        {
            return _name;
        }
        else
        {
            return "";
        }
    }
}
