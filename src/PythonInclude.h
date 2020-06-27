#pragma once

#pragma push_macro("slots")
#undef slots

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#include "numpy/arrayobject.h"
#define _DEBUG
#else
#include "Python.h"
#include "numpy/arrayobject.h"
#endif

#pragma pop_macro("slots")