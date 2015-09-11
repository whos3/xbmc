#pragma once
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

template<typename TType, bool(*TSetter)(TType, TType&) = nullptr>
class CTrackedProperty
{
public:
  CTrackedProperty()
    : m_value(),
      m_defaultValue(),
      m_changed(false)
  { }

  explicit CTrackedProperty(TType value)
    : m_value(value),
      m_defaultValue(value),
      m_changed(false)
  { }

  CTrackedProperty(const CTrackedProperty& trackedProperty)
    : m_value(trackedProperty.m_value),
      m_defaultValue(trackedProperty.m_defaultValue),
      m_changed(trackedProperty.m_changed)
  { }

  ~CTrackedProperty()
  { }

  // assignment operators
  inline CTrackedProperty& operator=(const CTrackedProperty& trackedProperty) { Set(trackedProperty.m_defaultValue); return *this; }
  inline CTrackedProperty& operator=(TType value) { Set(value); return *this; }

  // comparison operators
  inline bool operator==(const CTrackedProperty& rhs) const { return *this == rhs.m_value; }
  inline bool operator!=(const CTrackedProperty& rhs) const { return !(*this == rhs); }
  inline bool operator==(TType rhsValue) const { return m_value == rhsValue; }
  inline bool operator!=(TType rhsValue) const { return !operator==(rhsValue); }

  // implicit conversion
  inline operator TType() const { return m_value; }

  // getter / setter functions
  inline const TType& Get() const { return m_value; }

  inline bool IsDefault() const { return !m_changed; }

  bool Set(TType value)
  {
    if (TSetter == nullptr)
      m_value = value;
    else if (!TSetter(value, m_value))
      return false;

    UpdateChanged();
    return true;
  }

  inline void Reset() { Set(m_defaultValue); }

  // proxy for raw accessor
  class Proxy
  {
  private:
    friend class CTrackedProperty;

  public:
    // don't allow copy construction
    Proxy(const Proxy&) = delete;

    ~Proxy()
    {
      // make sure that the changed state is up-to-date
      m_property->UpdateChanged();
      m_property = nullptr;
    }

    // pointer access
    inline const TType& operator*() const { return m_property->m_value; }
    inline TType& operator*() { return m_property->m_value; }
    const TType& operator->() const { return m_property->m_value; }
    TType& operator->() { return m_property->m_value; }

  private:
    explicit Proxy(CTrackedProperty& trackedProperty)
      : m_property(&trackedProperty)
    { }

    CTrackedProperty* m_property;
  };

  // raw access
  inline Proxy Raw() { return Proxy(*this); }

private:
  friend class Proxy;

  inline void UpdateChanged() { m_changed = m_value != m_defaultValue; }

  TType m_value;
  TType m_defaultValue;

  bool m_changed;
};

// non-member operators
template<typename TType, bool(*TSetter)(TType, TType&) = nullptr>
inline bool operator==(TType lhsValue, const CTrackedProperty<TType, TSetter>& rhs) { return rhs == lhsValue; }

template<typename TType, bool(*TSetter)(TType, TType&) = nullptr>
inline bool operator!=(TType lhsValue, const CTrackedProperty<TType, TSetter>& rhs) { return rhs != lhsValue; }
