#include <gtest/gtest.h>
#include <gamecoe/component/parent_child.hpp>

using namespace gamecoe;

//==============================================================================
//                    ParentTests - parent component tests
//==============================================================================

class ParentTests : public ::testing::Test
{
protected:
    components::parent p;
};

//==============================================================================
//                        Default State
//==============================================================================

TEST_F(ParentTests, DefaultState)
{
    // Test 1: Default-constructed parent has no parent entity
    {
        EXPECT_EQ(p.parent, entity::invalid());
        EXPECT_FALSE(p.has_parent());
    }
}

//==============================================================================
//                        Assigning a Parent
//==============================================================================

TEST_F(ParentTests, AssignParent)
{
    // Test 1: Setting parent to a valid entity updates has_parent()
    {
        entity e = entity::create(5, 0);
        p.parent = e;

        EXPECT_TRUE(p.has_parent());
        EXPECT_EQ(p.parent, e);
    }
}

//==============================================================================
//                  ChildrenTests - children component tests
//==============================================================================

class ChildrenTests : public ::testing::Test
{
protected:
    components::children c;
};

//==============================================================================
//                        Default State
//==============================================================================

TEST_F(ChildrenTests, DefaultState)
{
    // Test 1: Default-constructed children has an empty list
    {
        EXPECT_TRUE(c.children.empty());
        EXPECT_FALSE(c.has_children());
    }
}

//==============================================================================
//                        Populating Children
//==============================================================================

TEST_F(ChildrenTests, PopulateChildren)
{
    entity e1 = entity::create(1, 0);
    entity e2 = entity::create(2, 0);

    // Test 1: Adding a single child updates has_children()
    {
        c.children.push_back(e1);

        EXPECT_TRUE(c.has_children());
        EXPECT_EQ(c.children.size(), 1);
    }

    // Test 2: Adding a second child preserves both entities, in order
    {
        c.children.push_back(e2);

        EXPECT_EQ(c.children.size(), 2);
        EXPECT_EQ(c.children[0], e1);
        EXPECT_EQ(c.children[1], e2);
    }
}
