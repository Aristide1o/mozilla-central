/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

/*

  A sample of XPConnect. This file contains the XPCOM factory the
  creates for SampleImpl objects.

*/

#include "nsCOMPtr.h"
#include "nscore.h"
#include "nsISample.h"
#include "nsIComponentManager.h"
#include "nsIServiceManager.h"
#include "nsXPComFactory.h"

/**
 * IIDs and CIDs (aka CLSIDs) are 32 digit hexadecimal numbers, called Globally
 * Unique IDs (GUIDs) as a whole.  They should only be generated by a trusted
 * guid generator.  GUID generatoration algorithms rely on parameters such as
 * the MAC address of your NIC, and the date/time of GUID creation to ensure
 * that no two GUIDs are ever the same.  The Windows program 'uuidgen' is one
 * way to create these numbers. The Unix alternative is probably out there
 * somewhere.
 *
 * NS_DEFINE_IID and NS_DEFINE_CID expand to define static IID/CID objects
 * that will be used later.  IID and CID objects are of the same form, so the
 * distinction between InterfaceID and ClassID is made strictly by what the
 * ID represents, and not by any binary differences.
 */
static NS_DEFINE_IID(kISupportsIID,        NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kIFactoryIID,         NS_IFACTORY_IID);
static NS_DEFINE_CID(kComponentManagerCID, NS_COMPONENTMANAGER_CID);
static NS_DEFINE_CID(kSampleCID,           NS_SAMPLE_CID);

/**
 * SampleFactoryImpl is a factory capable of creating nsSampleImpl objects.
 * It is the factory, as opposed to the class, which actually registers itself
 * with the XPCOM runtime, in a conversation that goes something like,
 * XPCOM: "Hello factory at <path-name>, what can you do for me?",
 * FACTORY: "Hello XPCOM, I can create the classes named <progID>:<CID>,
 * and <progID>:<CID> and ...".  This conversation is initiated when XPCOM calls
 * into the static |NSRegisterSelf| function of the shared library that houses this
 * factory.  Once this factory is properly registered, XPCOM can work it's magic.
 * After registration, any call into the Service Manager or Component Manager's
 * |CreateInstance| method requesting a progID or CID that this factory has
 * registered will cause XPCOM to call the static |NSGetFactory| function in
 * the .so, .dll, .whatever associated with this factory (if it hasn't already
 * done so), and then invoke the |CreateInstance| method of the resulting factory.
 */
class SampleFactoryImpl : public nsIFactory
{
public:
    SampleFactoryImpl(const nsCID &aClass, const char* className,
                      const char* progID);

    /**
     * This macro expands into a declaration of the nsISupports interface.
     * Every XPCOM component needs to implement nsISupports, as it acts
     * as the gateway to other interfaces this component implements.  You
     * could manually declare QueryInterface, AddRef, and Release instead
     * of using this macro, but why?
     */
    // nsISupports methods
    NS_DECL_ISUPPORTS

    // nsIFactory methods
    NS_IMETHOD CreateInstance(nsISupports *aOuter,
                              const nsIID &aIID,
                              void **aResult);

    NS_IMETHOD LockFactory(PRBool aLock);

protected:
    virtual ~SampleFactoryImpl();

protected:
    /**
     * When XPCOM calls |NSGetFactory|, it informs us what kind of class
     * it intends to create with this factory.  |CreateInstance| does NOT get
     * this information, so we need to record what kind of class |CreateInstance|
     * should return for later reference.
     */
    nsCID       mClassID;
    const char* mClassName;
    const char* mProgID;
};

////////////////////////////////////////////////////////////////////////

/**
 * The constructor needs to initailize reference counting and
 * record the CID / ProgID of the object it should return when
 * |CreateInstance| is called.
 */
SampleFactoryImpl::SampleFactoryImpl(const nsCID &aClass, 
                                     const char* className,
                                     const char* progID)
    : mClassID(aClass), mClassName(className), mProgID(progID)
{
    NS_INIT_REFCNT();
}

SampleFactoryImpl::~SampleFactoryImpl()
{
}

/**
 * This is what a normal implementation of |QueryInterface| (often
 * abbreviated QI) actually looks like. Because this QI only supports
 * nsISupports and nsIFactory, it, and the two NS_IMPL_* macros
 * that follow it could actually be replaced with the macro 
 * |NS_IMPL_ISUPPORTS(nsIFactory, nsIFactory::GetIID)| as nsSample does
 * in nsSample.cpp.
 * The XPCOM homepage (www.mozilla.org/projects/xpcom) has another reference
 * implementation of QueryInterface.
 */
NS_IMETHODIMP
SampleFactoryImpl::QueryInterface(const nsIID &aIID, void **aResult)
{
    if (! aResult)
        return NS_ERROR_NULL_POINTER;

    // Always NULL result, in case of failure
    *aResult = nsnull;

    if (aIID.Equals(kISupportsIID)) {
        *aResult = NS_STATIC_CAST(nsISupports*, this);
        AddRef();
        return NS_OK;
    } else if (aIID.Equals(kIFactoryIID)) {
        *aResult = NS_STATIC_CAST(nsIFactory*, this);
        AddRef();
        return NS_OK;
    }
    return NS_NOINTERFACE;
}

/*
 * These macros expand to implementations of AddRef and Release
 */
NS_IMPL_ADDREF(SampleFactoryImpl);
NS_IMPL_RELEASE(SampleFactoryImpl);

/**
 * The IID passed in here is for COM Aggregation.  Aggregation deals with
 * classes contained within other classes, a topic outside the scope of
 * this sample.
 *
 * Notice that this |CreateInstance| is very methodical.  It verifies that it
 * has been asked to create a supported class and calls QI on the newly
 * created object, verifying that the created class can actually support
 * the interface that was asked for.
 */
NS_IMETHODIMP
SampleFactoryImpl::CreateInstance(nsISupports *aOuter,
                                  const nsIID &aIID,
                                  void **aResult)
{
    if (! aResult)
        return NS_ERROR_NULL_POINTER;

    // Our example does not support COM Aggregation
    if (aOuter)
        return NS_ERROR_NO_AGGREGATION;

    *aResult = nsnull;

    nsresult rv;

    nsISupports *inst = nsnull;
    if (mClassID.Equals(kSampleCID)) {

        /* Try to create a new nsSampleImpl instance.  If this
           fails, then return the error code */
        if (NS_FAILED(rv = NS_NewSample((nsISample**) &inst)))
            return rv;
    }
    else {
        // We don't know how to create objects with the ClassID asked for
        return NS_ERROR_NO_INTERFACE;
    }

    if (! inst)
        return NS_ERROR_OUT_OF_MEMORY;

    if (NS_FAILED(rv = inst->QueryInterface(aIID, aResult))) {
        // We didn't get the right interface.
        NS_ERROR("didn't support the interface you wanted");
    }

    /*
     * We must call |Release| on the interface pointer that was
     * returned from |QueryInterface|.  The "interface release"
     * macro checks for a null pointer, calls |Release|, then
     * sets the pointer to null.  This macro should be
     * used instead of a direct |inst->Release()| call since
     * the macro records tracing information if
     * NS_BUILD_REFCNT_LOGGING is defined.
     */
    NS_IF_RELEASE(inst);
    return rv;
}

nsresult SampleFactoryImpl::LockFactory(PRBool aLock)
{
    // Not implemented in simplest case.
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////

/**
 * The XPCOM runtime will call this to get a new factory object for the
 * CID/progID it passes in.  XPCOM is responsible for caching the resulting
 * factory.
 */
// return the proper factory to the caller
extern "C" PR_IMPLEMENT(nsresult)
NSGetFactory(nsISupports* aServMgr,
             const nsCID &aClass,
             const char *aClassName,
             const char *aProgID,
             nsIFactory **aFactory)
{
    if (! aFactory)
        return NS_ERROR_NULL_POINTER;

    SampleFactoryImpl* factory = new SampleFactoryImpl(aClass, aClassName,
                                                       aProgID);
    if (factory == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;

    NS_ADDREF(factory);
    *aFactory = factory;
    return NS_OK;
}

/**
 * When the XPCOM runtime is initialized, it searches the component directory
 * for shared objects, and attempts to call |NSRegisterSelf| for each one it
 * encounters.
 *
 * Clients create instances of XPCOM objects using the ComponentManager.
 * In order for a Client to be able to create an instance of your
 * object, you must register your object and CLSID with the ComponentManager.
 *
 * The ServiceManager is used to access a "service", which is an object
 * that exists for the lifetime of the program.  Notice the distinction
 * between accessing a singleton object using the ServiceManager and
 * creating new instances using the ComponentManager.
 *
 * The ComponentManager is an example of an object which persists for
 * the length of the program.  To register with the ComponentManager
 * you first retrieve it using the ServiceManager.
 *
 * If you've got some spare time, and _really_ want to see whats going on
 * behind the scenes at registration time, soak up PlatformPrePopulateRegistry
 * (and all the functions it calls) in 
 * mozilla/xpcom/components/nsComponentManager.cpp
 *
 * It is possible to register that your component corresponds to a
 * ProgID, which is a human readable string such as
 * "component://netscape/image/decoder&type=image/gif".
 * Instead of using the 32 digit CLSID, Clients can use the
 * convienient ProgID.  ProgIDs are the preferred way of
 * accessing components.
 */

extern "C" PR_IMPLEMENT(nsresult)
NSRegisterSelf(nsISupports* aServMgr , const char* aPath)
{
    nsresult rv;

    nsCOMPtr<nsIServiceManager> servMgr(do_QueryInterface(aServMgr, &rv));
    if (NS_FAILED(rv)) return rv;

    NS_WITH_SERVICE(nsIComponentManager, compMgr, kComponentManagerCID, &rv);
    if (NS_FAILED(rv)) return rv;

    rv = compMgr->RegisterComponent(kSampleCID,
                                    "Sample World Component",
                                    "component://mozilla/sample/sample-world",
                                    aPath, PR_TRUE, PR_TRUE);

    if (NS_FAILED(rv)) return rv;

    return NS_OK;
}

/**
 * |NSUnregisterSelf| is responsible for undoing anything NSRegisterSelf does
 * to the registry.
 */
extern "C" PR_IMPLEMENT(nsresult)
NSUnregisterSelf(nsISupports* aServMgr, const char* aPath)
{
    nsresult rv;

    nsCOMPtr<nsIServiceManager> servMgr(do_QueryInterface(aServMgr, &rv));
    if (NS_FAILED(rv)) return rv;

    NS_WITH_SERVICE(nsIComponentManager, compMgr, kComponentManagerCID, &rv);
    if (NS_FAILED(rv)) return rv;

    rv = compMgr->UnregisterComponent(kSampleCID, aPath);
    if (NS_FAILED(rv)) return rv;

    return NS_OK;
}


