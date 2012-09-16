/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
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

import Helper
import SwigTypeParser

public class JavaScriptTools
{
   /**
    * This method gets the FULL class name as a variable including the 
    * namespace. If converts all of the '::' references to '_' so 
    * that the result can be used in part, or in whold, as a variable name
    */
   public static String getClassNameAsVariable(Node clazz) { return Helper.findFullClassName(clazz).replaceAll('::','_') }

   public static String getJSMethodName(Node method, MethodType methodType)
   {
      String clazz = Helper.findFullClassName(method)?.replaceAll('::','_')

      // if we're not in a class then this must be a method node
      assert (clazz != null || methodType == MethodType.method), 'Cannot use a non-class function as a constructor or destructor ' + method

      // it's ok to pass a 'class' node if the methodType is either constructor or destructor
      assert (method.name() != 'class' || (methodType == MethodType.constructor || methodType == MethodType.destructor))

      // if this is a constructor node then the methodtype best reflect that
      assert (method.name() != 'constructor' || methodType == MethodType.constructor), 'Cannot use a constructor node and not identify the type as a constructor' + method

      // if this is a destructor node then the methodtype best reflect that
      assert (method.name() != 'destructor' || methodType == MethodType.destructor), 'Cannot use a destructor node and not identify the type as a destructor' + method

      return (clazz == null) ? method.@sym_name :
      (
      // TODO: (methodType == MethodType.constructor) ? (clazz + "_New") :
      // TODO: (methodType == MethodType.destructor ? (clazz + "_Dealloc") : clazz + "_" + method.@sym_name)
      )
   }

  public static String makeDocString(Node docnode)
  { 
    if (docnode?.name() != 'doc')
      throw new RuntimeException("Invalid doc Node passed to JavaScriptTools.makeDocString (" + docnode + ")")

    String[] lines = (docnode.@value).split(Helper.newline)
    def ret = ''
    lines.eachWithIndex { val, index -> 
      val = ((val =~ /\\n/).replaceAll('')) // remove extraneous \n's 
      val = val.replaceAll("\\\\","\\\\\\\\") // escape backslash
      val = ((val =~ /\"/).replaceAll("\\\\\"")) // escape quotes
      ret += ('"' + val + '\\n"' + (index != lines.length - 1 ? Helper.newline : ''))
    }

    return ret
  }

  public static Node findValidBaseClass(Node clazz, Node module, boolean warn = false)
  {
    // I need to find the base type if there is a known class with it
    assert clazz.baselist.size() < 2, "${clazz} has multiple baselists - need to write code to separate out the public one."
    String baseclass = 'NULL'
    List knownbases = []
    if (clazz.baselist)
    { 
      if (clazz.baselist[0].base) clazz.baselist[0].base.each {
          Node baseclassnode = Helper.findClassNodeByName(module,it.@name,clazz)
          if (baseclassnode) knownbases.add(baseclassnode)
          else if (warn && !Helper.isKnownBaseType(it.@name,clazz))
            System.out.println("WARNING: the base class ${it.@name} for ${Helper.findFullClassName(clazz)} is unrecognized within ${module.@name}.")
        }
    }
    assert knownbases.size() < 2, 
      "The class ${Helper.findFullClassName(clazz)} has too many known base classes. Multiple inheritance isn't supported in the code generator. Please \"#ifdef SWIG\" out all but one."
    return knownbases.size() > 0 ? knownbases[0] : null
  }
}
