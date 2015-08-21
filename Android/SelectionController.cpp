// Generated from /Traceview/src/com/android/traceview/SelectionController.java
#include <com/android/traceview/SelectionController.hpp>

#include <java/lang/ClassCastException.hpp>
#include <java/util/ArrayList.hpp>

template<typename T, typename U>
static T java_cast(U* u)
{
    if(!u) return static_cast<T>(nullptr);
    auto t = dynamic_cast<T>(u);
    if(!t) throw new ::java::lang::ClassCastException();
    return t;
}

SelectionController::SelectionController(const ::default_init_tag&)
    : super(*static_cast< ::default_init_tag* >(0))
{
    clinit();
}

SelectionController::SelectionController()
    : SelectionController(*static_cast< ::default_init_tag* >(0))
{
    ctor();
}

void SelectionController::change(::java::util::ArrayList* selections, ::java::lang::Object* arg)
{
    this->mSelections = selections;
    setChanged();
    notifyObservers(arg);
}

java::util::ArrayList* SelectionController::getSelections()
{
    return java_cast< ::java::util::ArrayList* >(this->mSelections);
}

extern java::lang::Class *class_(const char16_t *c, int n);

java::lang::Class* SelectionController::class_()
{
    static ::java::lang::Class* c = ::class_(u"com.android.traceview.SelectionController", 41);
    return c;
}

java::lang::Class* SelectionController::getClass0()
{
    return class_();
}

