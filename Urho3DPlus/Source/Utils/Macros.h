
/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2014 Chukong Technologies
http://www.cocos2d-x.org
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
/// see  http://www.cocos2d-x.org  cocos2d-x / cocos / platform / CCPlatformMacros.h
#pragma once

// All Urho3D classes reside in namespace Urho3D
namespace Urho3D
{
	/** U_PROPERTY_READONLY is used to declare a protected variable.
	We can use getter to read the variable.
	@param varType the type of variable.
	@param varName variable name.
	@param funName "Get + funName" will be the name of the getter.
	@warning The getter is a public virtual function, you should rewrite it first.
	The variables and methods declared after U_PROPERTY_READONLY are all public.
	If you need protected or private, please declare.
	*/
#define U_PROPERTY_READONLY(varType, varName, funName)\
protected: varType varName; \
public: varType Get##funName(void) const;

#define U_PROPERTY_READONLY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName; \
public: const varType& Get##funName(void) const;

	/** U_PROPERTY is used to declare a protected variable.
	We can use getter to read the variable, and use the setter to change the variable.
	@param varType the type of variable.
	@param varName variable name.
	@param funName "Get + funName" will be the name of the getter.
	"Set + funName" will be the name of the setter.
	@warning The getter and setter are public virtual functions, you should rewrite them first.
	The variables and methods declared after U_PROPERTY are all public.
	If you need protected or private, please declare.
	*/
#define U_PROPERTY(varType, varName, funName)\
protected: varType varName; \
public: varType Get##funName(void); \
public: void Set##funName(varType var);

#define U_PROPERTY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName; \
public: const varType& Get##funName(void) const; \
public: void Set##funName(const varType& var);

	/** U_PROPERTY_IMP_READONLY is used to declare a protected variable.
	We can use getter to read the variable. Function is implemented.
	@param varType the type of variable.
	@param varName variable name.
	@param funName "Get + funName" will be the name of the getter.
	@warning The getter is a public inline function.
	The variables and methods declared after U_PROPERTY_IMP_READONLY are all public.
	If you need protected or private, please declare.
	*/
#define U_PROPERTY_IMP_READONLY(varType, varName, funName)\
protected: varType varName; \
public: varType Get##funName(void) const { return varName; }

#define U_PROPERTY_IMP_READONLY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName; \
public: const varType& Get##funName(void) const { return varName; }

	/** U_PROPERTY_IMP is used to declare a protected variable. Function is implemented.
	We can use getter to read the variable, and use the setter to change the variable.
	@param varType the type of variable.
	@param varName variable name.
	@param funName "Get + funName" will be the name of the getter.
	"Set + funName" will be the name of the setter.
	@warning The getter and setter are public inline functions.
	The variables and methods declared after U_PROPERTY_IMP are all public.
	If you need protected or private, please declare.
	*/
#define U_PROPERTY_IMP(varType, varName, funName)\
protected: varType varName; \
public: varType Get##funName(void) const { return varName; }\
public: void Set##funName(varType var){ varName = var; }

#define U_PROPERTY_IMP_PASS_BY_REF(varType, varName, funName)\
protected: varType varName; \
public: const varType& Get##funName(void) const { return varName; }\
public: void Set##funName(const varType& var){ varName = var; }


#define U_ATTRIBUTE_REF(varType, varName)\
protected: varType varName ## _; \
public: const varType& Get_##varName(void) const { return varName ## _; }\
public: void Set_##varName(const varType& value){ varName ## _ = value; }

#define U_ATTRIBUTE(varType, varName)\
protected: varType  varName ## _; \
public: varType Get_ ## varName(void) const { return varName ## _; }\
public: void Set_ ## varName(varType value){ varName ## _ = value; }

#define U_ATTRIBUTE_READ(varType, varName)\
protected: varType  varName ## _; \
public: varType Get_ ## varName(void) const { return varName ## _; }\

}
