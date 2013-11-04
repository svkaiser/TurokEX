#ifndef STAN_HULL_CONFIG_H

#define STAN_HULL_CONFIG_H

#include <assert.h>
#include <zone.h>

#ifdef STAN_HULL_USE_NAMESPACE
namespace physx
{
#endif
	typedef unsigned int PxU32;
	typedef int PxI32;
	typedef float PxF32;
	typedef unsigned char PxU8;

#define PX_ASSERT assert
#define PX_ALWAYS_ASSERT() assert(0)
#define PX_INLINE inline
#define PX_FORCE_PARAMETER_REFERENCE(_P) (void)(_P);
#define PX_ALLOC(x,y) Z_Malloc(x, PU_STATIC, 0)
#define PX_FREE(x) Z_Free(x)
#define PX_NEW(T) new(#T,__FILE__,__LINE__,sizeof(T)) T

	// An expression that should expand to nothing in _DEBUG builds.  
	// We currently use this only for tagging the purpose of containers for memory use tracking.
#if defined(_DEBUG)
#define PX_DEBUG_EXP(x) (x)
#define PX_DEBUG_EXP_C(x) x,
#else
#define PX_DEBUG_EXP(x)
#define PX_DEBUG_EXP_C(x)
#endif

	class UserAllocated
	{
	public:
		PX_INLINE void* operator new(size_t size,UserAllocated *t)
		{
			PX_FORCE_PARAMETER_REFERENCE(size);
			return t;
		}

		PX_INLINE void* operator new(size_t size,const char *className,const char* fileName, int lineno,size_t classSize)
		{
			PX_FORCE_PARAMETER_REFERENCE(className);
			PX_FORCE_PARAMETER_REFERENCE(fileName);
			PX_FORCE_PARAMETER_REFERENCE(lineno);
			PX_FORCE_PARAMETER_REFERENCE(classSize);
			return PX_ALLOC(size,className);
		}

		inline void* operator new[](size_t size,const char *className,const char* fileName, int lineno,size_t classSize)
		{
			PX_FORCE_PARAMETER_REFERENCE(className);
			PX_FORCE_PARAMETER_REFERENCE(fileName);
			PX_FORCE_PARAMETER_REFERENCE(lineno);
			PX_FORCE_PARAMETER_REFERENCE(classSize);
			return PX_ALLOC(size,className);
		}

		inline void  operator delete(void* p,UserAllocated *t)
		{
			PX_FORCE_PARAMETER_REFERENCE(p);
			PX_FORCE_PARAMETER_REFERENCE(t);
			PX_ALWAYS_ASSERT(); // should never be executed
		}

		inline void  operator delete(void* p)
		{
			PX_FREE(p);
		}

		inline void  operator delete[](void* p)
		{
			PX_FREE(p);
		}

		inline void  operator delete(void *p,const char *className,const char* fileName, int line,size_t classSize)
		{
			PX_FORCE_PARAMETER_REFERENCE(className);
			PX_FORCE_PARAMETER_REFERENCE(fileName);
			PX_FORCE_PARAMETER_REFERENCE(line);
			PX_FORCE_PARAMETER_REFERENCE(classSize);
			PX_FREE(p);
		}

		inline void  operator delete[](void *p,const char *className,const char* fileName, int line,size_t classSize)
		{
			PX_FORCE_PARAMETER_REFERENCE(className);
			PX_FORCE_PARAMETER_REFERENCE(fileName);
			PX_FORCE_PARAMETER_REFERENCE(line);
			PX_FORCE_PARAMETER_REFERENCE(classSize);
			PX_FREE(p);
		}

	};



#ifdef STAN_HULL_USE_NAMESPACE
};
#endif

#endif
