/* -*-Mode: C++;-*-
 * $Id: xdfs.h,v 1.6 2000/11/10 05:26:28 jmacd Exp $
 *
 * Copyright (C) 1998, 1999, 2000, Joshua P. MacDonald
 * <jmacd@CS.Berkeley.EDU> and The Regents of the University of
 * California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 *    Neither name of The University of California nor the names of
 *    its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SLP_H__
#define __SLP_H__

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstddef>
#include <cassert>

#define M_div_2          ((M)>>1)
#define M_minus_1        ((M)-1)
#define M_minus_2        ((M)-2)
#define M_div_2_minus_1  (((M)>>1)-1)
#define M_div_2_minus_2  (((M)>>1)-2)

#define SLP_MAX_COUNT    (M_minus_1)
#define SLP_MIN_COUNT    (M_div_2_minus_1)

#ifndef	MAX
#define	MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

enum SlpNodeType {
    SlpLeaf     = 0,
    SlpInternal = 1
};

enum SlpDuplicateFlag {
    SlpNoDuplicates    = 0,
    SlpAllowDuplicates = 1
};

template <class Key, class Data, Key max_key, int M, int dups>
class SlpNode {
public:

#define _slp_leaf  _u._head._u_leaf
#define _slp_count _u._head._u_count
#define _slp_right _u._head._u_right

#define slp_down(x,i)  (x)->_du[(i)]._l_down
#define slp_data(x,i)  (x)->_du[(i)]._l_data
#define slp_keys(x,i)  (x)->_ku[(i)]._k
#define slp_debug(x,i) (x)->_dbg[(i)]

    typedef union {
	SlpNode *_l_down;
	Data     _l_data;
	Key      _k;
    } DataUnion;

    union {
	struct {
	    // true if node is a leaf
	    uint8_t    _u_leaf;

	    // node occupancy
	    uint8_t    _u_count;

	    // duplicate bitfields (M <= 16)
	    //uint32_t     _u_dups : 16;

	    // right pointer
	    SlpNode *_u_right;
	} _head;

	//DataUnion _pad[2];
    } _u;

#ifdef DEBUG_SLP
    uint _dbg[SLP_MAX_COUNT];

    void print_node (const char* desc)
    {
	std::cout << "Printing keys for " << desc << std::endl;
	for (int i = 0; i < _slp_count; i += 1) {
	    std::cout << slp_keys(this,i) << std::endl;
	}
    }
#endif

    DataUnion _ku[SLP_MAX_COUNT];
    DataUnion _du[SLP_MAX_COUNT];
};

template <class Key, class Data, Key max_key, int M, int dups> class SlpIterator;

template <class Key, class Data, Key max_key, int M, int dups>
class Slp
{
public:

    friend class SlpIterator<Key,Data,max_key,M,dups>;

    typedef SlpIterator<Key,Data,max_key,M,dups> iterator;
    typedef SlpNode<Key,Data,max_key,M,dups>     Node;

    Slp ()
	: _node_count (0),
	  _size       (0),
	  _height     (0),
	  _free_list  (NULL)
    {
	// this assertion is due to the width of the duplicates bitfield
	//assert (M <= 16);

	// invariant: left_leaf is always the leftmost leaf.  if there
	// are no internal nodes, it is also the root.
	_left_leaf = _head = slp_node_new ();

	_head->_slp_count   = 1;
	slp_keys(_head, 0)  = max_key;
	_head->_slp_leaf    = 1;
	_head->_slp_right   = NULL;
    }

    ~Slp ()
    {
        Node *n;
        while (_free_list != NULL) {
            n = _free_list;
            _free_list = _free_list->_slp_right;
            delete n;
        }
    }

public:

    // Iterators
    iterator begin      () const;
    iterator search_gub (Key k) const;
    std::pair<iterator, bool> search_llb (Key k) const;
    std::pair<iterator, bool> search_lub (Key k) const;

    // Dictionary functions.
    bool slp_insert  (Key k, Data  d);
    bool slp_remove  (Key k, Data *dp);
    bool slp_lookup  (Key k, Data *dp);
    std::pair<iterator, bool> slp_lookup_iter (Key k);

    // Priority queue functions.
    bool slp_lookup_min (Key *kp, Data *dp);
    bool slp_remove_min (Key *kp, Data *dp);

    Key  slp_max_key (void) const { return max_key; }
    int  slp_size    (void) const { return _size; }
    bool slp_empty   (void) const { return _size == 0; }
    int64_t slp_mem_size (void) const {
        return static_cast<int64_t>(_node_count) * static_cast<int64_t>(_node_size);
    }

#ifdef DEBUG_SLP
    void  slp_print_compute_row (Node *node);
    uint  slp_down_all          (Node *node, int i);
    void  slp_print_node        (Node *node);
    void  slp_print             ();
#endif

private:

    void slp_node_free (Node *n)
    {
	n->_slp_right = _free_list;
	_free_list = n;
    _node_count--;
    }

    Node* slp_node_new (void)
    {
	Node* n;
	if (_free_list) {
	    n = _free_list;
	    _free_list = n->_slp_right;
	} else {
	    n = new Node;
        _node_count++;
	}
	return n;
    }

    void  slp_shift_keys_right (Node *x, int i);
    void  slp_shift_keys_left  (Node *x, int i);
    Node* slp_split_node       (Node *x);

    void  slp_concat_nodes            (Node *l, Node *r);
    void  slp_redistribute_right_left (Node* l, Node* r);
    void  slp_redistribute_left_right (Node* l, Node* r);

private:

    static const int _node_size = sizeof (Node);

    int         _node_count;
    int         _size;
    int         _height;

    Node       *_free_list;
    Node       *_head;
    Node       *_left_leaf;
};

// SLP Iterator
template <class Key, class Data, Key max_key, int M, int dups>
class SlpIterator
{
public:

    friend class Slp<Key,Data,max_key,M,dups>;

    typedef Slp<Key,Data,max_key,M,dups>         Base;
    typedef SlpNode<Key,Data,max_key,M,dups>     Node;
    typedef SlpIterator<Key,Data,max_key,M,dups> Self;

    Key key() const {
        //assert (! end ());
        return slp_keys (_node, _index);
    }
    Data data () const {
        //assert (! end ());
        return slp_data (_node, _index);
    }
    void data (Data other) {
        //assert (! end ());
        slp_data (_node, _index) = other;
    }
    bool end () const {
        return (_node->_slp_right == NULL) && (_index == (_node->_slp_count-1));
    }

    Self& operator++ ()
    {
	if (! end ()) {
	    _index += 1;

	    if ((_index == _node->_slp_count) && (_node->_slp_right != NULL)) {
		_index = 0;
		_node  = _node->_slp_right;
	    }
	}

	return *this;
    }

private:

    SlpIterator (const Base *base)
	: _node  (base->_left_leaf),
	  _index (0)
    {
    }

    SlpIterator (const Base *base, Node *node, int index)
	: _node  (node),
	  _index (index)
    {
    }

    Node *_node;
    int   _index;
};

template <class Key, class Data, Key max_key, int M, int dups>
inline SlpIterator<Key,Data,max_key,M,dups>
Slp<Key,Data,max_key,M,dups>::begin () const
{
    return iterator (this);
}

// This macro moves the key, data, and duplicate bitfield
// from one location to another, used in all of the node
// rearrangement functions below
#define SLP_SHIFT(fn,from,tn,to)                         \
    slp_keys((tn),(to)) = slp_keys((fn),(from));         \
    slp_data((tn),(to)) = slp_data((fn),(from))

// Shift keys to the right, making room for a new element
template <class Key, class Data, Key max_key, int M, int dups>
inline void
Slp<Key,Data,max_key,M,dups>::slp_shift_keys_right (Node *x,
						    int   i)
{
    for (int j = x->_slp_count - 1; j >= i; j -= 1) {
	SLP_SHIFT (x, j, x, j+1);
    }

    x->_slp_count += 1;
}

// Shift keys to the left, deleting an element
template <class Key, class Data, Key max_key, int M, int dups>
inline void
Slp<Key,Data,max_key,M,dups>::slp_shift_keys_left (Node *x,
						   int   i)
{
    for (; i < x->_slp_count - 1; i += 1) {
	SLP_SHIFT (x, i+1, x, i);
    }

    x->_slp_count -= 1;
}

// Split a node into two nodes, balancing keys between them
template <class Key, class Data, Key max_key, int M, int dups>
inline SlpNode<Key,Data,max_key,M,dups>*
Slp<Key,Data,max_key,M,dups>::slp_split_node (Node* node)
{
    Node* new_node = slp_node_new ();
    int i;

    new_node->_slp_leaf  = node->_slp_leaf;
    new_node->_slp_right = node->_slp_right;
    new_node->_slp_count = M_div_2_minus_1;
    node->_slp_right = new_node;
    node->_slp_count = M_div_2;

    for (i = M_minus_2; i >= M_div_2; i -= 1) {
	SLP_SHIFT (node, i, new_node, i-M_div_2);
    }

    return new_node;
}

// Reverse-split a node, two nodes into one
template <class Key, class Data, Key max_key, int M, int dups>
inline void
Slp<Key,Data,max_key,M,dups>::slp_concat_nodes (Node *l,
						Node *r)
{
    int i, j;

    l->_slp_right = r->_slp_right;
    j = l->_slp_count;

    for (i = 0; i < r->_slp_count; i += 1, j += 1) {
	SLP_SHIFT (r, i, l, j);
    }

    l->_slp_count += r->_slp_count;

    slp_node_free (r);
}

// Rebalance the keys between two nodes, moving right to left
template <class Key, class Data, Key max_key, int M, int dups>
inline void
Slp<Key,Data,max_key,M,dups>::slp_redistribute_right_left (Node* l,
							   Node* r)
{
    int lc = l->_slp_count;
    int rc = r->_slp_count;
    int move = rc - ((lc + rc) >> 1);
    int i, j;

    for (i = 0, j = lc; i < move; i += 1, j += 1) {
	SLP_SHIFT (r, i, l, j);
    }

    for (j = 0; i < rc; i += 1, j += 1) {
	SLP_SHIFT (r, i, r, j);
    }

    r->_slp_count = rc - move;
    l->_slp_count = lc + move;
}

// Rebalance the keys between two nodes, moving left to right
template <class Key, class Data, Key max_key, int M, int dups>
inline void
Slp<Key,Data,max_key,M,dups>::slp_redistribute_left_right (Node* l,
							   Node* r)
{
    int lc = l->_slp_count;
    int rc = r->_slp_count;
    int move = lc - ((lc + rc) >> 1);
    int i, j;

    for (i = rc-1, j = i+move; i >= 0; i -= 1, j -= 1) {
	SLP_SHIFT (r, i, r, j);
    }

    for (i = lc - move, j = 0; i < lc; i += 1, j += 1) {
	SLP_SHIFT (l, i, r, j);
    }

    r->_slp_count = rc + move;
    l->_slp_count = lc - move;
}

template <class Key, class Data, Key max_key, int M, int dups>
inline bool
Slp<Key,Data,max_key,M,dups>::slp_insert (Key  key,
					  Data value)
{
    Node *new_child, *child, *head = _head, *x = head;
    int i;

    //assert (key < max_key);

    for (;;) {

	if (x->_slp_leaf) {

	    // If the node is a leaf, the loop is more
	    // complicated than below for an internal node,
	    // due to the terminating max_key
	    for (i = 0; i < x->_slp_count; i += 1) {
		Key ki = slp_keys(x,i);

		if (key > ki) {
		    continue;
		} else if (key == ki) {
		    // The key was found
		    return false;
		} else {
		    break;
		}
	    }

	    // The key was not found, and i is positioned at
	    // its correct location.  Shift the keys right to
	    // make room.
	    slp_shift_keys_right (x, i);

	    // Insert the new key
	    _size += 1;
	    slp_keys(x,i) = key;
	    slp_data(x,i) = value;

	    goto done;
	} else {

	    // This loop is simpler than above, due to
	    // the max_key
	    for (i = 0; key > slp_keys(x,i); i += 1) {
		// Do nothing
	    }

	    // This verifies the max_key loop-simplification
	    //assert (i < x->_slp_count);

	    // Here, i is the index of the next child node to
	    // search.
	    child = slp_down(x,i);

	    if (child->_slp_count == SLP_MAX_COUNT) {

		// If the child is full (and since the next iteration
		// may need to add a new key), split it now.  This is
		// what makes the Slp a "top-down" method, and allows
		// it to be iterative rather than recursive.
		new_child = slp_split_node (child);

		// Shift keys, making room for the new node
		slp_shift_keys_right (x, i);

		// Insert the new node.  The old key moves right,
		// and the new key is the last key of the left-split
		// node, which is the same node as child.
		slp_keys(x,i) = slp_keys(child,M_div_2_minus_1);

		// The children change.
		slp_down(x,i) = child;
		slp_down(x,i+1) = new_child;

		// If the search key resides in the right-split node,
		// update child appropriately.
		if (key > slp_keys(x,i)) {
		    child = slp_down(x,i+1);
		}
	    }

	    // Descend and repeat.
	    x = child;
	}
    }

  done:

    if (head->_slp_count == SLP_MAX_COUNT) {

	// If the root node needs to be split, create a new internal
	// node.
	child = head;
	new_child = slp_split_node (head);
	head = _head = slp_node_new ();

	_height += 1;

	head->_slp_count  = 2;
	head->_slp_leaf   = 0;
	head->_slp_right  = NULL;

	slp_keys(head,1)  = slp_keys(new_child, M_div_2_minus_2);
	slp_keys(head,0)  = slp_keys(child, M_div_2_minus_1);
	slp_down(head,1)  = new_child;
	slp_down(head,0)  = child;
    }

    return true;
}

template <class Key, class Data, Key max_key, int M, int dups>
inline bool
Slp<Key,Data,max_key,M,dups>::slp_lookup (Key key, Data *dp)
{
    Node *x = _head;
    int i;

    //assert (key < max_key);

    for (;;) {

	if (x->_slp_leaf) {

	    for (i = 0; i < x->_slp_count; i += 1) {
		Key ki = slp_keys(x,i);

		if (key > ki) {
		    continue;
		} else if (key == ki) {

		    if (dp) {
			(* dp) = slp_data(x,i);
		    }

		    return true;
		} else {
		    break;
		}
	    }

	    return false;

	} else {

	    for (i = 0; key > slp_keys(x,i); i += 1) {
		/* nothing */
	    }

	    //assert (i < x->_slp_count);

	    x = slp_down(x,i);
	}
    };
}

template <class Key, class Data, Key max_key, int M, int dups>
inline std::pair<SlpIterator<Key,Data,max_key,M,dups>, bool>
Slp<Key,Data,max_key,M,dups>::slp_lookup_iter (Key key)
{
    Node *x = _head;
    int i;

    //assert (key < max_key);

    for (;;) {

	if (x->_slp_leaf) {

	    for (i = 0; i < x->_slp_count; i += 1) {
		Key ki = slp_keys(x,i);

		if (key > ki) {
		    continue;
		} else if (key == ki) {
		    return std::pair<SlpIterator<Key,Data,max_key,M,dups>, bool>(iterator (this, x, i), true);
		} else {
		    break;
		}
	    }

	    return std::pair<SlpIterator<Key,Data,max_key,M,dups>, bool>(iterator (this), false);

	} else {

	    for (i = 0; key > slp_keys(x,i); i += 1) {
		/* nothing */
	    }

	    //assert (i < x->_slp_count);

	    x = slp_down(x,i);
	}
    };
}

// Search for the greatest element less than or equal to KEY
template <class Key, class Data, Key max_key, int M, int dups>
inline SlpIterator<Key,Data,max_key,M,dups>
Slp<Key,Data,max_key,M,dups>::search_gub (Key key) const
{
    Node *x = _head;
    int i;

    //assert (key < max_key);

    for (;;) {

	if (x->_slp_leaf) {

	    for (i = 0; i < x->_slp_count; i += 1) {
		Key ki = slp_keys(x,i);

		if (key > ki) {
		    continue;
		}
		// KEY <= KI
		else if (key == ki) {
		    return iterator (this, x, i);
		}
		// KEY < KI, use previous value
		else if (i > 0) {
		    // Its on same node
		    return iterator (this, x, i-1);
		} else if (x == _left_leaf) {
		    // First key
		    return iterator (this, x, 0);
		} else {
		    // Beginning of a node, repeat search
		    return search_gub (key-1);
		}
	    }

	} else {

	    for (i = 0; key > slp_keys(x,i); i += 1) {
		/* nothing */
	    }

	    //assert (i < x->_slp_count);

	    x = slp_down(x,i);
	}
    };
}

// Search for the smallest element greater than or equal to KEY
template <class Key, class Data, Key max_key, int M, int dups>
inline std::pair<SlpIterator<Key,Data,max_key,M,dups>, bool>
Slp<Key,Data,max_key,M,dups>::search_llb (Key key) const
{
    Node *x = _head;
    int i;

    //assert (key < max_key);

    for (;;) {

	if (x->_slp_leaf) {

	    for (i = 0; i < x->_slp_count; i += 1) {
		Key ki = slp_keys(x,i);

		if (key > ki) {
		    continue;
		}
		// KEY <= KI
		else if (ki != max_key) {
		    return std::pair<iterator, bool>(iterator (this, x, i), true);
		}
	    }

        return std::pair<iterator, bool>(iterator (this), false);

	} else {

	    for (i = 0; key > slp_keys(x,i); i += 1) {
		/* nothing */
	    }

	    //assert (i < x->_slp_count);

	    x = slp_down(x,i);
	}
    };
}

// Search for the smallest element greater than KEY
template <class Key, class Data, Key max_key, int M, int dups>
inline std::pair<SlpIterator<Key,Data,max_key,M,dups>, bool>
Slp<Key,Data,max_key,M,dups>::search_lub (Key key) const
{
    Node *x = _head;
    int i;

    //assert (key < max_key);

    for (;;) {

	if (x->_slp_leaf) {

	    for (i = 0; i < x->_slp_count; i += 1) {
		Key ki = slp_keys(x,i);

		if (key > ki) {
		    continue;
		}
		// KEY <= KI
		else if (ki != max_key) {
		    return std::pair<iterator, bool>(iterator (this, x, i), true);
		}
	    }

        return std::pair<iterator, bool>(iterator (this), false);

	} else {

	    for (i = 0; key > slp_keys(x,i); i += 1) {
		/* nothing */
	    }

	    //assert (i < x->_slp_count);

	    x = slp_down(x,i);
	}
    };
}

template <class Key, class Data, Key max_key, int M, int dups>
inline bool
Slp<Key,Data,max_key,M,dups>::slp_lookup_min (Key *kp, Data *dp)
{
    //assert (_left_leaf->_slp_count != 0);

    if (_left_leaf->_slp_count == 1) {
	return false;
    }

    if (kp) {
	(* kp) = slp_keys (_left_leaf, 0);
    }

    if (dp) {
	(* dp) = slp_data (_left_leaf, 0);
    }

    return true;
}

template <class Key, class Data, Key max_key, int M, int dups>
inline bool
Slp<Key,Data,max_key,M,dups>::slp_remove_min (Key *kp, Data *dp)
{
    Key k;

    if (! kp) {
	kp = & k;
    }

    if (! slp_lookup_min (kp, dp)) {
	return false;
    }

    return slp_remove (* kp, dp);
}

template <class Key, class Data, Key max_key, int M, int dups>
inline bool
Slp<Key,Data,max_key,M,dups>::slp_remove (Key key, Data *dp)
{
    Node *child, *head = _head, *x = head, *rchild, *lchild;
    bool deleted = false;
    int i;

    //assert (key < max_key);

    for (;;) {

	if (x->_slp_leaf) {

	    for (i = 0; i < x->_slp_count; i += 1) {
		Key ki = slp_keys(x,i);

		if (key > ki) {
		    continue;
		} else if (key == ki) {

		    if (dp) {
			(*dp) = slp_data(x,i);
		    }

		    slp_shift_keys_left (x, i);

		    _size -= 1;

		    deleted = true;
		    break;
		} else {
		    break;
		}
	    }

	    goto done;
	} else {

	    for (i = 0; key > slp_keys(x,i); i += 1) {
		/* nothing */
	    }

	    //assert (i < x->_slp_count);

	    child = slp_down (x,i);

	    if (child->_slp_count == SLP_MIN_COUNT) {
		if (i == 0) {
		    rchild = slp_down(x,i+1);

		    if (rchild->_slp_count == SLP_MIN_COUNT) {
			slp_shift_keys_left (x, i);

			slp_concat_nodes (child, rchild);

			slp_down(x,i) = child;
		    } else {
			slp_redistribute_right_left (child, rchild);

			slp_keys(x,i) = slp_keys(child, child->_slp_count-1);
		    }
		} else {
		    lchild = slp_down(x,i-1);

		    if (lchild->_slp_count == SLP_MIN_COUNT) {
			slp_shift_keys_left (x, i-1);

			slp_concat_nodes (lchild, child);

			slp_down(x,i-1) = lchild;

			child = lchild;
		    } else {
			slp_redistribute_left_right (lchild, child);

			slp_keys (x, i-1) = slp_keys (lchild, lchild->_slp_count-1);
		    }
		}
	    }

	    x = child;
	}
    }
  done:

    if ((head->_slp_count == 1) && (! head->_slp_leaf)) {
	_head = slp_down (head, 0);
	slp_node_free (head);
	_height -= 1;
    }

    return deleted;
}

#ifdef DEBUG_SLP
template <class Key, class Data, Key max_key, int M, int dups>
inline void
Slp<Key,Data,max_key,M,dups>::slp_print_compute_row (Node *node)
{
    Node *row;
    int col_pos = 0, i;

    if (! node->_slp_leaf) {
	slp_print_compute_row (slp_down (node, 0));
    } else {

	for (row = node; row; row = row->_slp_right) {
	    col_pos += 1;
	    for (i = 0; i < row->_slp_count; i += 1) {
		slp_debug(row,i) = col_pos;
		col_pos += 6;
	    }
	    col_pos += 1;
	}
    }
}

template <class Key, class Data, Key max_key, int M, int dups>
inline uint
Slp<Key,Data,max_key,M,dups>::slp_down_all (Node *node, int i)
{
    while (! node->_slp_leaf) {
	node = slp_down (node, i);
	i = node->_slp_count-1;
    }

    return slp_debug(node, i);
}

template <class Key, class Data, Key max_key, int M, int dups>
inline void
Slp<Key,Data,max_key,M,dups>::slp_print_node (Node *node)
{
    Node *row;
    int col_pos = 0, i;

    do {
	col_pos = 0;
	for (row = node; row; row = row->_slp_right) {
	    std::cout << "[";
	    col_pos += 1;
	    for (i = 0; i < row->_slp_count; i += 1) {
		int this_pos;

		this_pos = slp_down_all (row, i);

		if (col_pos < this_pos) {
            std::cout << std::setw(this_pos - col_pos) << "";
		}

		col_pos = this_pos + 6;

		if (slp_keys(row,i) == max_key) {
            std::cout << std::setw(6) << "Max";
		} else {
            std::cout << std::setw(6) << slp_keys (row,i);
		}
	    }
	    std::cout << "]";
	    col_pos += 1;
	}
	std::cout << std::endl;
    } while ((! node->_slp_leaf) && (node = slp_down(node,0)));
    std::cout << std::endl;
}

template <class Key, class Data, Key max_key, int M, int dups>
inline void
Slp<Key,Data,max_key,M,dups>::slp_print ()
{
    slp_print_compute_row (_head);
    slp_print_node        (_head);
}

#endif

#endif /* __SLP_H__ */
/*
  Local Variables:
  mode: c++
  End:
 */
