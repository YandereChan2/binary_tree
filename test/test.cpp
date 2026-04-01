#include "gtest/gtest.h"

#include "binary_tree.h"

TEST(binary_tree, default_construct)
{
    Yc::binary_tree<int> tree;
    EXPECT_TRUE(tree.empty());

    EXPECT_FALSE((bool)tree.root());
    EXPECT_TRUE(tree.root().null());
    EXPECT_TRUE(tree.root().valid());
    
    EXPECT_FALSE((bool)tree.croot());
    EXPECT_TRUE(tree.croot().null());
    EXPECT_TRUE(tree.croot().valid());

    EXPECT_FALSE((bool)tree.nroot());
    EXPECT_TRUE(tree.nroot().null());

    EXPECT_FALSE((bool)tree.cnroot());
    EXPECT_TRUE(tree.cnroot().null());
}