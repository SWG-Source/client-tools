package com.sony.soe.swg.cts.ui;

import java.util.Enumeration;
import java.util.Vector;

import javax.swing.tree.TreeNode;

import com.sony.soe.swg.cts.*;

/**
 * TreeNode which knows how to display object data.
 * @author bmauer
 *
 */
public class ObjectTreeNode implements TreeNode
{
    private   SWGObject        _object;
    protected Vector _children;
    private   TreeNode          _parent;
    
    ObjectTreeNode( SWGObject object, TreeNode parent )
    {
        _object = object;
        _parent = parent;
        _children = new Vector();
        if ( _object.getTemplateId() != -1 )
        {
            StringTreeNode template = new StringTreeNode( "Template", new String[] { _object.getTemplateName() + " (" + _object.getTemplateId() + ")" }, this );
            _children.add( template );
        }
        if ( _object.containsCount() > 0 )
        {
            StringTreeNode contains = new StringTreeNode( "Contains", _object.getContains(), this );
            _children.add( contains );
        }
        if ( _object.objvarCount() > 0 )
        {
            StringTreeNode objvars = new StringTreeNode( "Object Variables", _object.getObjvars(), this );
            _children.add( objvars );
        }
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
        if ( _object != null )
        {
            return _object.getName() + " (" + _object.getId() + ")";
        }
        else
        {
            return "";
        }
    }
}
