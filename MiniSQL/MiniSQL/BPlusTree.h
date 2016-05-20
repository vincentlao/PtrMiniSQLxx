#pragma once

#include "BufferManager.h"
#include "IndexManager.h"

template<typename TKey>
class BPlusTree
{
public:
    using key_type = TKey;
    using ptr_type = BlockPtr;
    const static size_t key_size = sizeof(key_type);
    const static size_t ptr_size = sizeof(ptr_type);
    const static size_t key_count = (BufferBlock::BlockSize
                                     - ptr_size
                                     - sizeof(size_t)
                                     - sizeof(size_t)
                                     - sizeof(bool)
                                     - sizeof(BlockPtr)
                                     ) / (key_size + ptr_size);
    const static size_t ptr_count = key_count;
    const static size_t next_index = ptr_count;
    struct BTreeNodeModel
    {
        bool is_leaf;
        size_t total_key;
        size_t total_ptr;
        ptr_type parent;
        key_type keys[key_count];
        ptr_type ptrs[ptr_count];
    };
private:
    template<typename T>
    class OberservableArray
    {
    private:
        T* _array;
        size_t* _psize;
        size_t _capacity;
    public:
        OberservableArray(T* base, size_t& size, size_t capacity)
            : _array(base)
            , _psize(&size)
            , _capacity(capacity)
        {
        }
        T& operator[](size_t i) { assert(i < *_psize); return _array[i]; }
        T* begin() { return _array; }
        T* end() { return _array + *_psize; }
        size_t capacity() const { return _capacity; }
        size_t& size() { return *_psize; }
        const size_t& size() const { return *_psize; }
    };

    class TreeNode
    {
        friend class BPlusTree<TKey>;
        ptr_type _selfPtr;
        BTreeNodeModel* _base;
        OberservableArray<key_type> _keys;
        OberservableArray<ptr_type> _ptrs;
    public:
        bool is_leaf() { return _base->is_leaf; }
        size_t& key_count() { return _base->total_key; }
        size_t& ptr_count() { return _base->total_ptr; }

        TreeNode parent() { return TreeNode(_base->parent); }
        ptr_type& parent_ptr() { return _base->parent; }

        TreeNode next() { assert(is_leaf()); return TreeNode(_base->ptrs[next_index]); }
        ptr_type& next_ptr() { assert(is_leaf()); return _base->ptrs[next_index]; }

        ptr_type self_ptr() const { return _selfPtr; }

        OberservableArray<key_type>& keys() { return _keys; }
        OberservableArray<ptr_type>& ptrs() { return _ptrs; }
        void reset(bool is_leaf = false)
        {
            _base->is_leaf = is_leaf;
            _base->total_key = 0;
            _base->total_ptr = 0;
            _base->parent = nullptr;
        }
        explicit TreeNode(ptr_type blockPtr)
            : _selfPtr(blockPtr)
            , _base(blockPtr->as<BTreeNodeModel>())
            , _keys(_base->keys, _base->total_key, key_count)
            , _ptrs(_base->ptrs, _base->totak_ptr, ptr_count)
        {
        }
        explicit TreeNode(BufferBlock& block)
            : TreeNode(block.ptr())
        {
        }
        void insert_after_ptr(size_t i, const key_type& key, const ptr_type& ptr)
        {

        }
        void insert_before_ptr(size_t i, const ptr_type& ptr, const key_type& key)
        {
            
        }
        void insert_after_key(size_t i, const ptr_type& ptr, const key_type& key)
        {
            
        }
        void insert_before_key(size_t i, const key_type& key, const ptr_type& ptr)
        {
            
        }
        void remove_entry(const key_type& key)
        {
            
        }
    };
    static_assert(sizeof(BTreeNodeModel) <= BufferBlock::BlockSize, "sizeof(BTreeNode) > BufferBlock::BlockSize");

private:
    BlockPtr _root;

    TreeNode find_leaf(const TKey& key)
    {
        TreeNode node(_root);
        while (!node.is_leaf())
        {
            auto iterFirstGTKey = std::find_if(node.keys().begin(), node.keys().end(), [&key](const key_type& ckey) {
                return ckey >= key;
            });
            auto iFirstGTKey = iterFirstGTKey - node.keys().begin();
            if (iterFirstGTKey == node.keys().end()) //iFirstGTKey == end
            {
                auto iLastNonNullptr = std::find_if(std::make_reverse_iterator(node.ptrs().end()),
                                                    std::make_reverse_iterator(node.ptrs().begin()),
                                                    [](const ptr_type& cptr) {
                    return cptr != ptr_type(nullptr);
                });
                node = TreeNode(*iLastNonNullptr);
            }
            else if (key == node.keys()[iFirstGTKey])
            {

                node = TreeNode(node.ptrs()[iFirstGTKey + 1]);
            }
            else
            {
                node = TreeNode(node.ptrs()[iFirstGTKey]);
            }
        }
        return node;
    }

    BlockPtr find(const TKey& key)
    {
        TreeNode targetLeaf = find_leaf(key);

        auto iterValue = std::find(targetLeaf.keys().begin(), targetLeaf.keys().end(), [&key](const key_type& ckey) {
            return key = ckey;
        });
        auto iValue = iterValue - targetLeaf.key().begin();
        if (iterValue != targetLeaf.keys().end())
        {
            return targetLeaf.ptrs()[iValue];
        }
        return BlockPtr(nullptr);
    }

    static void insert_leaf(TreeNode& leaf, const TKey& key, const BlockPtr& ptr)
    {
        /*
        auto place = std::find_if(leaf.keys().begin(), leaf.keys().end(), [&key](const key_type& ckey) {
            return ckey > key;
        });
        auto offset = place - leaf.keys().begin();
        assert(leaf.keys().begin() + offset + 1 < leaf.keys().begin() + key_count);
        std::move(leaf.keys().begin() + offset, leaf.keys().end(), leaf.keys().begin() + offset + 1);
        std::move(leaf.ptrs().begin() + offset, leaf.ptrs().end(), leaf.ptrs().begin() + offset + 1);

        leaf.keys()[offset] = key;
        leaf.ptrs()[offset] = pValue;
        ++leaf.keys().count();
        ++leaf.ptrs().count();
        */
        if (key < leaf.keys()[0])
        {
            leaf.insert_before_ptr(0, ptr, key);
        }
        else
        {
            auto place = std::find_if(leaf.keys().begin(), leaf.keys().end(), [&key](const key_type& ckey) {
                return ckey > key;
            });
            auto iInsertAfter = place - leaf.keys().begin() - 1;
            leaf.insert_after_key(iInsertAfter, ptr, key);
        }
    }
    static key_type find_min_key(const TreeNode& leaf)
    {
        assert(leaf.is_leaf());
        assert(leaf.keys().count() > 0);
        key_type minKey = leaf.keys()[0];
        for (auto& key : leaf.keys())
        {
            if (key < minKey)
            {
                minKey = key;
            }
        }
        return minKey;
    }

    template<typename TContainer, typename T>
    static void sorted_insert(TContainer& cont, const T& value)
    {
        auto place = std::find_if(cont.begin(), cont.end(), [&value](const T& cvalue) {
            return cvalue > value;
        });
        place_insert(cont, place - cont.begin(), value);
    }

    template<typename TContainer, typename T>
    static void place_insert(TContainer& cont, size_t place, const T& value)
    {
        cont.count() += 1;
        std::move(cont.begin() + place, cont.end(), cont.begin() + place + 1);
        cont[place] = value;
    }

    void insert_parent(TreeNode& node, const key_type& key, const BlockPtr& ptr)
    {
        if (node.parent_ptr() == nullptr) //node is _root
        {
            auto newIndex = IndexManager::instance().allocate();
            auto& newBlock = BufferManager::instance().find_or_alloc(newIndex.first, newIndex.second);
            TreeNode newRoot(newBlock);
            newRoot.reset(false);
            newRoot.keys().count() = 1;
            newRoot.keys()[0] = key;
            newRoot.ptrs().count() = 2;
            newRoot.ptrs()[0] = node.self_ptr();
            newRoot.ptrs()[1] = ptr;
            _root = newBlock.ptr();
            return;
        }
        //not root
        TreeNode p = node.parent();
        if (p.ptrs().count() < ptr_count)
        {
            
            auto iterInsertAfter = std::find(p.ptrs().begin(), p.ptrs().end(), node.self_ptr());
            auto iInsertAfter = iterInsertAfter - p.ptrs().begin();
            p.insert_adter_ptr(iInsertAfter, key, ptr);
            /*
            p.ptrs().count() += 1;
            std::move(p.ptrs().begin() + iNode, p.ptrs().end(), p.ptrs().begin() + iNode + 1);
            p.ptrs()[iNode] = ptr;

            p.keys().count() += 1;
            std::move(p.keys().begin() + iNode, p.keys().end(), p.keys().begin() + iNode + 1);
            p.keys()[iNode] = key;
            */
        }
        else
        {
            auto pair = insert_split_node(p, key, ptr);
            insert_parent(p, pair.second, pair.first.self_ptr());
        }
    }
    
    void remove(const key_type& key, const ptr_type& ptr)
    {
        auto leaf = find_leaf(key);
        remove_entry(leaf, key, ptr);
    }

    void remove_entry(TreeNode& node, const key_type& key, const ptr_type& ptr)
    {
        node.remove_entry(key);
        if(node->parent_ptr() == nullptr && node->ptr_count() == 1)
        {
            _root = node->ptrs()[0];
            node->parent_ptr() = nullptr;

        }
    }

    static std::pair<TreeNode, key_type> insert_split_node(TreeNode& node, const key_type& key, const ptr_type& ptr)
    {
        
    }

    static TreeNode insert_split_leaf(TreeNode& node, const key_type& key, const ptr_type& ptr)
    {
        
    }

    void insert(const TKey& key, const BlockPtr& ptr)
    {
        TreeNode targetLeaf = find_leaf(key);
        if (targetLeaf.count() < key_count)
        {
            insert_leaf(targetLeaf, key, ptr);
        }
        else
        {
            TreeNode newNode = insert_split_leaf(targetLeaf, key, ptr);
            insert_parent(targetLeaf,newNode.keys()[0], newNode.self_ptr())
            /*
            assert(targetLeaf.count() == key_count);
            auto newIndex = IndexManager::instance().allocate();
            auto& newBlock = BufferManager::instance().find_or_alloc(newIndex.first, newIndex.second);
            TreeNode newLeaf(newBlock);
            newLeaf.reset(true);

            std::copy(targetLeaf.keys().begin(), targetLeaf.keys().end(), newLeaf.keys());
            std::copy(targetLeaf.ptrs().begin(), targetLeaf.ptrs().end(), newLeaf.ptrs());
            newLeaf.count() = key_count - 1;
            insert_leaf(newLeaf, key, pValue);

            newLeaf->next_ptr() = targetLeaf->next_ptr();
            targetLeaf->next_ptr() = newBlock;

            std::copy(newLeaf.ptrs().begin(), newLeaf.ptrs().begin() + key_count / 2, targetLeaf.ptrs());
            targetLeaf.ptrs().count() = key_count / 2;
            std::copy(newLeaf.keys().begin(), newLeaf.keys().begin() + key_count / 2, targetLeaf.keys());
            targetLeaf.ptrs().count() = key_count / 2;

            std::copy(newLeaf.ptrs().begin() + key_count / 2, newLeaf.ptrs().end(), newLeaf.ptrs());
            newLeaf.ptrs().count() = key_count / 2;
            std::copy(newLeaf.keys().begin() + key_count / 2, newLeaf.keys().end(), newLeaf.keys());
            newLeaf.ptrs().count() = key_count / 2;

            key_type minKey = find_min_key(newLeaf);
            insert_parent(targetLeaf, minKey, newBlock.ptr());
            */

        }
    }

};

