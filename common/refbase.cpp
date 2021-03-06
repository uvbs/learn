/*
* Copyright (C) 2005 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <assert.h>
#include "CLogger.h"
#include "refbase.h"

#pragma warning(disable:4355)

#define TAG "<RefBase>"

// compile with refcounting debugging enabled
#define DEBUG_REFS                      0
#define DEBUG_REFS_ENABLED_BY_DEFAULT   1
#define DEBUG_REFS_CALLSTACK_ENABLED    1

// log all reference counting operations
#define PRINT_REFS                      0

// ---------------------------------------------------------------------------

namespace android
{ 
	class Mutex
	{
	public:
		void lock() const {m.lock();}
		void unlock() const {m.unlock();}
	private:
		mutable tthread::mutex m;
	};
	
	static inline Mutex& gloableAtomicMutex()
	{
		static Mutex* m = new Mutex;
		return *m;
	}

	// ---------------------------------------------------------------------------
	int atomic_add(int a, volatile int* b, const Mutex* m)
	{
		int rt;

		m = m ? m : &gloableAtomicMutex();

		m->lock();
		rt = *b;
		(*b) += a;
		m->unlock();

		return rt;
	}

	int atomic_cmpxchg(int oldVal, volatile int newVal, volatile int *v, const Mutex* m)
	{
		int rt;

		m = m ? m : &gloableAtomicMutex();

		m->lock();
		if(*v == oldVal)
		{
			*v = newVal;
			rt = 0;
		}
		else 
		{
			rt = 1;
		}

		m->unlock();

		return rt;

	}

	int atomic_or(int a, volatile int* b, const Mutex* m)
	{
		int rt;

		m = m ? m : &gloableAtomicMutex();

		m->lock();
		rt = *b;
		(*b) |= a;
		m->unlock();

		return rt;
	}

	int atomic_and(int a, volatile int* b, Mutex* m)
	{
		int rt;

		m = m ? m : &gloableAtomicMutex();

		m->lock();
		rt = *b;
		(*b) &= a;
		m->unlock();

		return rt;
	}

#define INITIAL_STRONG_VALUE (1<<28)

	// ---------------------------------------------------------------------------

	RefBase::Destroyer::~Destroyer() {
	}

	// ---------------------------------------------------------------------------

	class RefBase::weakref_impl : public RefBase::weakref_type
	{
	public:
		volatile int    mStrong;
		volatile int    mWeak;
		RefBase* const      mBase;
		volatile int    mFlags;
		Destroyer*          mDestroyer;

#if !DEBUG_REFS

		weakref_impl(RefBase* base)
			: mStrong(INITIAL_STRONG_VALUE)
			, mWeak(0)
			, mBase(base)
			, mFlags(0)
			, mDestroyer(0)
		{
		}

		void addStrongRef(const void* /*id*/) { }
		void removeStrongRef(const void* /*id*/) { }
		void addWeakRef(const void* /*id*/) { }
		void removeWeakRef(const void* /*id*/) { }
		void printRefs() const { }
		void trackMe(bool, bool) { }

#else

		weakref_impl(RefBase* base)
			: mStrong(INITIAL_STRONG_VALUE)
			, mWeak(0)
			, mBase(base)
			, mFlags(0)
			, mStrongRefs(NULL)
			, mWeakRefs(NULL)
			, mTrackEnabled(!!DEBUG_REFS_ENABLED_BY_DEFAULT)
			, mRetain(false)
		{
			//LOGI("NEW weakref_impl %p for RefBase %p", this, base);
		}

		~weakref_impl()
		{
			LOG_ALWAYS_FATAL_IF(!mRetain && mStrongRefs != NULL, "Strong references remain!");
			LOG_ALWAYS_FATAL_IF(!mRetain && mWeakRefs != NULL, "Weak references remain!");
		}

		void addStrongRef(const void* id)
		{
			addRef(&mStrongRefs, id, mStrong);
		}

		void removeStrongRef(const void* id)
		{
			if (!mRetain)
				removeRef(&mStrongRefs, id);
			else
				addRef(&mStrongRefs, id, -mStrong);
		}

		void addWeakRef(const void* id)
		{
			addRef(&mWeakRefs, id, mWeak);
		}

		void removeWeakRef(const void* id)
		{
			if (!mRetain)
				removeRef(&mWeakRefs, id);
			else
				addRef(&mWeakRefs, id, -mWeak);
		}

		void trackMe(bool track, bool retain)
		{ 
			mTrackEnabled = track;
			mRetain = retain;
		}

		void printRefs() const
		{
			String8 text;

			{
				AutoMutex _l(const_cast<weakref_impl*>(this)->mMutex);

				char buf[128];
				sprintf(buf, "Strong references on RefBase %p (weakref_type %p):\n", mBase, this);
				text.append(buf);
				printRefsLocked(&text, mStrongRefs);
				sprintf(buf, "Weak references on RefBase %p (weakref_type %p):\n", mBase, this);
				text.append(buf);
				printRefsLocked(&text, mWeakRefs);
			}

			{
				char name[100];
				snprintf(name, 100, "/data/%p.stack", this);
				int rc = open(name, O_RDWR | O_CREAT | O_APPEND);
				if (rc >= 0) {
					write(rc, text.string(), text.length());
					close(rc);
					LOGD("STACK TRACE for %p saved in %s", this, name);
				}
				else LOGE("FAILED TO PRINT STACK TRACE for %p in %s: %s", this,
					name, strerror(errno));
			}
		}

	private:
		struct ref_entry
		{
			ref_entry* next;
			const void* id;
#if DEBUG_REFS_CALLSTACK_ENABLED
			CallStack stack;
#endif
			int32_t ref;
		};

		void addRef(ref_entry** refs, const void* id, int32_t mRef)
		{
			if (mTrackEnabled) {
				AutoMutex _l(mMutex);
				ref_entry* ref = new ref_entry;
				// Reference count at the time of the snapshot, but before the
				// update.  Positive value means we increment, negative--we
				// decrement the reference count.
				ref->ref = mRef;
				ref->id = id;
#if DEBUG_REFS_CALLSTACK_ENABLED
				ref->stack.update(2);
#endif

				ref->next = *refs;
				*refs = ref;
			}
		}

		void removeRef(ref_entry** refs, const void* id)
		{
			if (mTrackEnabled) {
				AutoMutex _l(mMutex);

				ref_entry* ref = *refs;
				while (ref != NULL) {
					if (ref->id == id) {
						*refs = ref->next;
						delete ref;
						return;
					}

					refs = &ref->next;
					ref = *refs;
				}

				LOG_ALWAYS_FATAL("RefBase: removing id %p on RefBase %p (weakref_type %p) that doesn't exist!",
					id, mBase, this);
			}
		}

		void printRefsLocked(String8* out, const ref_entry* refs) const
		{
			char buf[128];
			while (refs) {
				char inc = refs->ref >= 0 ? '+' : '-';
				sprintf(buf, "\t%c ID %p (ref %u):\n", 
					inc, refs->id, refs->ref);
				out->append(buf);
#if DEBUG_REFS_CALLSTACK_ENABLED
				out->append(refs->stack.toString("\t\t"));
#else
				out->append("\t\t(call stacks disabled)");
#endif
				refs = refs->next;
			}
		}

		Mutex mMutex;
		ref_entry* mStrongRefs;
		ref_entry* mWeakRefs;

		bool mTrackEnabled;
		// Collect stack traces on addref and removeref, instead of deleting the stack references
		// on removeref that match the address ones.
		bool mRetain;

#if 0
		void addRef(KeyedVector<const void*, int32_t>* refs, const void* id)
		{
			AutoMutex _l(mMutex);
			ssize_t i = refs->indexOfKey(id);
			if (i >= 0) {
				++(refs->editValueAt(i));
			} else {
				i = refs->add(id, 1);
			}
		}

		void removeRef(KeyedVector<const void*, int32_t>* refs, const void* id)
		{
			AutoMutex _l(mMutex);
			ssize_t i = refs->indexOfKey(id);
			LOG_ALWAYS_FATAL_IF(i < 0, "RefBase: removing id %p that doesn't exist!", id);
			if (i >= 0) {
				int32_t val = --(refs->editValueAt(i));
				if (val == 0) {
					refs->removeItemsAt(i);
				}
			}
		}

		void printRefs(const KeyedVector<const void*, int32_t>& refs)
		{
			const size_t N=refs.size();
			for (size_t i=0; i<N; i++) {
				printf("\tID %p: %u remain\n", refs.keyAt(i), refs.valueAt(i));
			}
		}

		mutable Mutex mMutex;
		KeyedVector<const void*, int32_t> mStrongRefs;
		KeyedVector<const void*, int32_t> mWeakRefs;
#endif

#endif
	};

	// ---------------------------------------------------------------------------

	void RefBase::incStrong(const void* id) const
	{
		weakref_impl* const refs = mRefs;
		refs->addWeakRef(id);
		refs->incWeak(id);

		refs->addStrongRef(id);
		const int c = atomic_inc(&refs->mStrong);
		if(!(c > 0))
		{
			assert(0);
			Log.i(TAG, "incStrong() called on %p after last strong ref", refs);
		}
#if PRINT_REFS
		Log.d("incStrong of %p from %p: cnt=%u\n", this, id, c);
#endif
		if (c != INITIAL_STRONG_VALUE)  {
			return;
		}

		atomic_add(-INITIAL_STRONG_VALUE, &refs->mStrong);
		const_cast<RefBase*>(this)->onFirstRef();
	}

	void RefBase::decStrong(const void* id) const
	{
		weakref_impl* const refs = mRefs;
		refs->removeStrongRef(id);
		const int c = atomic_dec(&refs->mStrong);
#if PRINT_REFS
		Log.d("decStrong of %p from %p: cnt=%u\n", this, id, c);
#endif
		if(!(c >= 1))
		{
			assert(0);		
			Log.i(TAG, "decStrong() called on %p too many times", refs);
		}

		if (c == 1) 
		{
			const_cast<RefBase*>(this)->onLastStrongRef(id);
			if ((refs->mFlags&OBJECT_LIFETIME_WEAK) != OBJECT_LIFETIME_WEAK) {
				if (refs->mDestroyer) {
					refs->mDestroyer->destroy(this);
				} else {
					delete this;
				}
			}
		}
		refs->removeWeakRef(id);
		refs->decWeak(id);
	}

	void RefBase::forceIncStrong(const void* id) const
	{
		weakref_impl* const refs = mRefs;
		refs->addWeakRef(id);
		refs->incWeak(id);

		refs->addStrongRef(id);
		const int c = atomic_inc(&refs->mStrong);
		if(!(c >= 0))
		{
			assert(0);
			Log.i(TAG, "forceIncStrong called on %p after ref count underflow", refs);
		}
#if PRINT_REFS
		Log.d("forceIncStrong of %p from %p: cnt=%u\n", this, id, c);
#endif

		switch (c) {
	case INITIAL_STRONG_VALUE:
		atomic_add(-INITIAL_STRONG_VALUE, &refs->mStrong);
		// fall through...
	case 0:
		const_cast<RefBase*>(this)->onFirstRef();
		}
	}

	int RefBase::getStrongCount() const
	{
		return mRefs->mStrong;
	}

	void RefBase::setDestroyer(RefBase::Destroyer* destroyer) {
		mRefs->mDestroyer = destroyer;
	}

	RefBase::Destroyer* RefBase::getDestroyer() const {
		return mRefs->mDestroyer;
	}

	RefBase* RefBase::weakref_type::refBase() const
	{
		return static_cast<const weakref_impl*>(this)->mBase;
	}

	void RefBase::weakref_type::incWeak(const void* id)
	{
		weakref_impl* const impl = static_cast<weakref_impl*>(this);
		impl->addWeakRef(id);
		const int c = atomic_inc(&impl->mWeak);
		if(!(c >= 0))
		{
			assert(0);		
			Log.i(TAG, "incWeak called on %p after last weak ref", this);
		}
	}

	void RefBase::weakref_type::decWeak(const void* id)
	{
		weakref_impl* const impl = static_cast<weakref_impl*>(this);
		impl->removeWeakRef(id);
		const int c = atomic_dec(&impl->mWeak);
		if(!(c >= 1))
		{
			assert(0);		
			Log.i(TAG, "decWeak called on %p too many times", this);
		}

		if (c != 1) 
			return;

		if ((impl->mFlags&OBJECT_LIFETIME_WEAK) != OBJECT_LIFETIME_WEAK) {
			if (impl->mStrong == INITIAL_STRONG_VALUE) {
				if (impl->mBase) {
					if (impl->mDestroyer) {
						impl->mDestroyer->destroy(impl->mBase);
					} else {
						delete impl->mBase;
					}
				}
			} else {
				// LOGV("Freeing refs %p of old RefBase %p\n", this, impl->mBase);
				delete impl;
			}
		} else {
			impl->mBase->onLastWeakRef(id);
			if ((impl->mFlags&OBJECT_LIFETIME_FOREVER) != OBJECT_LIFETIME_FOREVER) {
				if (impl->mBase) {
					if (impl->mDestroyer) {
						impl->mDestroyer->destroy(impl->mBase);
					} else {
						delete impl->mBase;
					}
				}
			}
		}
	}

	bool RefBase::weakref_type::attemptIncStrong(const void* id)
	{
		incWeak(id);

		weakref_impl* const impl = static_cast<weakref_impl*>(this);

		int curCount = impl->mStrong;
		if(!(curCount >= 0))
		{
			assert(0);		
			Log.i(TAG, "attemptIncStrong called on %p after underflow", this);
		}
		while (curCount > 0 && curCount != INITIAL_STRONG_VALUE) {
			if (atomic_cmpxchg(curCount, curCount+1, &impl->mStrong) == 0) {
				break;
			}
			curCount = impl->mStrong;
		}

		if (curCount <= 0 || curCount == INITIAL_STRONG_VALUE) {
			bool allow;
			if (curCount == INITIAL_STRONG_VALUE) {
				// Attempting to acquire first strong reference...  this is allowed
				// if the object does NOT have a longer lifetime (meaning the
				// implementation doesn't need to see this), or if the implementation
				// allows it to happen.
				allow = (impl->mFlags&OBJECT_LIFETIME_WEAK) != OBJECT_LIFETIME_WEAK
					|| impl->mBase->onIncStrongAttempted(FIRST_INC_STRONG, id);
			} else {
				// Attempting to revive the object...  this is allowed
				// if the object DOES have a longer lifetime (so we can safely
				// call the object with only a weak ref) and the implementation
				// allows it to happen.
				allow = (impl->mFlags&OBJECT_LIFETIME_WEAK) == OBJECT_LIFETIME_WEAK
					&& impl->mBase->onIncStrongAttempted(FIRST_INC_STRONG, id);
			}
			if (!allow) {
				decWeak(id);
				return false;
			}
			curCount = atomic_inc(&impl->mStrong);

			// If the strong reference count has already been incremented by
			// someone else, the implementor of onIncStrongAttempted() is holding
			// an unneeded reference.  So call onLastStrongRef() here to remove it.
			// (No, this is not pretty.)  Note that we MUST NOT do this if we
			// are in fact acquiring the first reference.
			if (curCount > 0 && curCount < INITIAL_STRONG_VALUE) {
				impl->mBase->onLastStrongRef(id);
			}
		}

		impl->addWeakRef(id);
		impl->addStrongRef(id);

#if PRINT_REFS
		Log.d("attemptIncStrong of %p from %p: cnt=%u\n", this, id, curCount);
#endif

		if (curCount == INITIAL_STRONG_VALUE) {
			atomic_add(-INITIAL_STRONG_VALUE, &impl->mStrong);
			impl->mBase->onFirstRef();
		}

		return true;
	}

	bool RefBase::weakref_type::attemptIncWeak(const void* id)
	{
		weakref_impl* const impl = static_cast<weakref_impl*>(this);

		int curCount = impl->mWeak;
		if(!(curCount >= 0))
		{
			assert(0);		
			Log.i(TAG, "attemptIncWeak called on %p after underflow", this);
		}
		while (curCount > 0) {
			if (atomic_cmpxchg(curCount, curCount+1, &impl->mWeak) == 0) {
				break;
			}
			curCount = impl->mWeak;
		}

		if (curCount > 0) {
			impl->addWeakRef(id);
		}

		return curCount > 0;
	}

	int RefBase::weakref_type::getWeakCount() const
	{
		return static_cast<const weakref_impl*>(this)->mWeak;
	}

	void RefBase::weakref_type::printRefs() const
	{
		static_cast<const weakref_impl*>(this)->printRefs();
	}

	void RefBase::weakref_type::trackMe(bool enable, bool retain)
	{
		static_cast<weakref_impl*>(this)->trackMe(enable, retain);
	}

	RefBase::weakref_type* RefBase::createWeak(const void* id) const
	{
		mRefs->incWeak(id);
		return mRefs;
	}

	RefBase::weakref_type* RefBase::getWeakRefs() const
	{
		return mRefs;
	}

	RefBase::RefBase()
		: mRefs(new weakref_impl(this))
	{
		//    LOGV("Creating refs %p with RefBase %p\n", mRefs, this);
	}

	RefBase::~RefBase()
	{
		if ((mRefs->mFlags & OBJECT_LIFETIME_WEAK) == OBJECT_LIFETIME_WEAK) {
			if (mRefs->mWeak == 0) {
				delete mRefs;
			}
		}
	}

	void RefBase::extendObjectLifetime(int mode)
	{
		atomic_or(mode, &mRefs->mFlags);
	}

	void RefBase::onFirstRef()
	{
	}

	void RefBase::onLastStrongRef(const void* /*id*/)
	{
	}

	bool RefBase::onIncStrongAttempted(unsigned int flags, const void* id)
	{
		return (flags&FIRST_INC_STRONG) ? true : false;
	}

	void RefBase::onLastWeakRef(const void* /*id*/)
	{
	}

}; // namespace android
