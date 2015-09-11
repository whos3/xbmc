/*
 *      Copyright (C) 2015 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "utils/TrackedProperty.h"

#include "gtest/gtest.h"

template<typename TType>
bool DummySetter(TType newValue, TType& value)
{
  value = newValue;
  return true;
}

template<typename TType>
bool BadSetter(TType newValue, TType& value)
{
  return false;
}

TEST(TestTrackedProperty, TrackedPropertyConstruction)
{
  CTrackedProperty<int> prop;

  CTrackedProperty<int> propInit(0);
  ASSERT_EQ(0, propInit.Get());
  ASSERT_TRUE(propInit.IsDefault());

  CTrackedProperty<int> propCopy(propInit);
  ASSERT_EQ(propInit, propCopy);
  ASSERT_EQ(0, propCopy.Get());
  ASSERT_TRUE(propCopy.IsDefault());
}

TEST(TestTrackedProperty, TrackedPropertyConstructionWithSetter)
{
  CTrackedProperty<int, DummySetter<int>> prop;

  CTrackedProperty<int, DummySetter<int>> propInit(0);
  ASSERT_EQ(0, propInit.Get());
  ASSERT_TRUE(propInit.IsDefault());
  ASSERT_TRUE(propInit.Set(1));
  ASSERT_EQ(1, propInit.Get());
  ASSERT_FALSE(propInit.IsDefault());
}

TEST(TestTrackedProperty, TrackedPropertyConstructionWithBadSetter)
{
  CTrackedProperty<int, BadSetter<int>> propBad(0);
  ASSERT_EQ(0, propBad.Get());
  ASSERT_TRUE(propBad.IsDefault());
  ASSERT_FALSE(propBad.Set(1));
  ASSERT_EQ(0, propBad.Get());
  ASSERT_TRUE(propBad.IsDefault());
}

TEST(TestTrackedProperty, TrackedPropertyCopying)
{
  CTrackedProperty<int, DummySetter<int>> propInit(0);
  ASSERT_TRUE(propInit.Set(1));

  CTrackedProperty<int, DummySetter<int>> propCopy(propInit);
  ASSERT_EQ(propInit, propCopy);
  ASSERT_EQ(1, propCopy.Get());
  ASSERT_FALSE(propCopy.IsDefault());
}

TEST(TestTrackedProperty, TrackedPropertyAssignment)
{
  CTrackedProperty<int> prop0(0);
  CTrackedProperty<int> prop1(1);

  ASSERT_EQ(2, prop0 = 2);
  ASSERT_EQ(2, prop0);

  ASSERT_EQ(1, prop0 = prop1);
  ASSERT_EQ(1, prop0);
}

TEST(TestTrackedProperty, TrackedPropertyComparison)
{
  CTrackedProperty<int> prop0(0);
  CTrackedProperty<int> prop1(1);

  ASSERT_TRUE(prop0 == 0);
  ASSERT_TRUE(0 == prop0);
  ASSERT_FALSE(prop0 == 1);
  ASSERT_FALSE(1 == prop0);
  ASSERT_FALSE(prop0 == prop1);
  ASSERT_FALSE(prop1 == prop0);

  ASSERT_TRUE(prop1 != 0);
  ASSERT_TRUE(0 != prop1);
  ASSERT_FALSE(prop1 != 1);
  ASSERT_FALSE(1 != prop1);
  ASSERT_TRUE(prop0 != prop1);
  ASSERT_TRUE(prop1 != prop0);
}

TEST(TestTrackedProperty, TrackedPropertyGetter)
{
  CTrackedProperty<int> prop(0);
  ASSERT_EQ(0, prop);
  ASSERT_EQ(0, prop.Get());
}

TEST(TestTrackedProperty, TrackedPropertySetter)
{
  CTrackedProperty<int> prop(0);
  ASSERT_EQ(0, prop);
  ASSERT_EQ(1, prop = 1);
  ASSERT_EQ(2, (prop = 2).Get());
  ASSERT_EQ(2, prop);
  ASSERT_TRUE(prop.Set(3));
  ASSERT_EQ(3, prop);
}

TEST(TestTrackedProperty, TrackedPropertyDefault)
{
  CTrackedProperty<int> prop(0);
  ASSERT_EQ(0, prop);
  ASSERT_TRUE(prop.IsDefault());

  prop = 1;
  ASSERT_FALSE(prop.IsDefault());

  prop = 0;
  ASSERT_TRUE(prop.IsDefault());

  prop = 1;
  ASSERT_FALSE(prop.IsDefault());
  prop.Reset();
  ASSERT_TRUE(prop.IsDefault());
}

TEST(TestTrackedProperty, TrackedPropertyRawAccess)
{
  CTrackedProperty<std::vector<int>> prop;
  ASSERT_TRUE(prop.Raw()->empty());
  ASSERT_TRUE(prop.IsDefault());

  prop.Raw()->push_back(1);
  ASSERT_FALSE(prop.IsDefault());
}
