// Generated from /Traceview/src/com/android/traceview/SelectionController.java

#pragma once

#include <com/android/traceview/fwd-Traceview.hpp>
#include <java/lang/fwd-Traceview.hpp>
#include <java/util/fwd-Traceview.hpp>
#include <java/util/Observable.hpp>

struct default_init_tag;

class SelectionController
    : public ::java::util::Observable
{

public:
    typedef ::java::util::Observable super;

private:
    ::java::util::ArrayList* mSelections {  };

public:
    virtual void change(::java::util::ArrayList* selections, ::java::lang::Object* arg);
    virtual ::java::util::ArrayList* getSelections();

    // Generated
    SelectionController();
protected:
    SelectionController(const ::default_init_tag&);


public:
    static ::java::lang::Class *class_();

private:
    virtual ::java::lang::Class* getClass0();
};
