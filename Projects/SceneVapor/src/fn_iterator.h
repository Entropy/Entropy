#pragma once
#include <vector>
#include <functional>
#include <cstdio>
#include <limits>

namespace fn{

template<typename IT, typename T>
class filter;
template<typename IT, typename T, typename U>
class map;
template<typename IT, typename T, typename U>
class filter_map;
template<typename IT, typename T>
class enumerate;
template<typename IT, typename T>
class take;
template<typename IT, typename T>
class skip;
template<typename IT, typename T, typename OtherIT, typename U>
class flat_map;
template<typename IT, typename T, typename OtherIT, typename U>
class zip;
template<typename IT, typename T>
class dedup;

template<typename T, class Derived, class BaseIT>
class base_iterator: public std::iterator<std::input_iterator_tag, T>{
public:
	typedef T type_name;
	/*virtual T & operator*() = 0;
	virtual const T & operator*() const = 0;
    virtual const BaseIT & it() const = 0;
    virtual BaseIT & it() = 0;
    virtual Derived begin() const = 0;
    virtual Derived end() const = 0;
	virtual Derived& operator++() = 0;
	virtual Derived operator++(int) {Derived tmp(static_cast<Derived&>(*this)); operator++(); return tmp;}
    virtual Derived operator+(size_t count) const = 0;
	virtual Derived & operator+=(size_t count) = 0;*/

    Derived operator++(int) {Derived tmp(static_cast<const Derived&>(*this)); operator++(); return tmp;}

    template<typename OtherIt>
    bool operator==(const OtherIt& rhs) const {return static_cast<const Derived&>(*this).it() == rhs.it();}

    template<typename OtherIt>
    bool operator!=(const OtherIt& rhs) const {return static_cast<const Derived&>(*this).it() != rhs.it();}

    template<typename OtherIt>
    bool operator<(const OtherIt& rhs) const {return static_cast<const Derived&>(*this).it() < rhs.it();}

    template<typename OtherIt>
    bool operator>(const OtherIt& rhs) const {return static_cast<const Derived&>(*this).it() > rhs.it();}

    template<typename OtherIt>
    bool operator<=(const OtherIt& rhs) const {return static_cast<const Derived&>(*this).it() <= rhs.it();}

    template<typename OtherIt>
    bool operator>=(const OtherIt& rhs) const {return static_cast<const Derived&>(*this).it() >= rhs.it();}

    bool operator==(const BaseIT& rhs) const {return static_cast<const Derived&>(*this).it() == rhs;}
    bool operator!=(const BaseIT& rhs) const {return static_cast<const Derived&>(*this).it() != rhs;}
    bool operator<(const BaseIT& rhs) const {return static_cast<const Derived&>(*this).it() < rhs;}
    bool operator>(const BaseIT& rhs) const {return static_cast<const Derived&>(*this).it() > rhs;}
    bool operator<=(const BaseIT& rhs) const {return static_cast<const Derived&>(*this).it() <= rhs;}
    bool operator>=(const BaseIT& rhs) const {return static_cast<const Derived&>(*this).it() >= rhs;}


    /// Returns an enumerate iterator which contains the value
    /// from the underlying collection + an index starting at 0
    fn::enumerate<Derived,T> enumerate();

    /// Returns only the elements for which the predicate passed
    /// as argument returns true
    fn::filter<Derived,T> filter(std::function<bool(const T&)> func);

    /// Maps the underlying iterator to a different type through
    /// the predicate passed as parameter which receives an argument
    /// of the type of the current iterator and returns the mapped type
    template<typename F>
    auto map(F func) -> fn::map<Derived,T,decltype(func(std::declval<T>()))>;

    /// Simultaneously filters and maps the underlying iterator
    /// to a different type through the predicate passed as parameter
    /// which receives an argument of the type of the current iterator
    /// and returns an option<U>. Anything that returns none will be
    /// filtered out
    template<typename F>
    auto filter_map(F func) -> fn::filter_map<Derived,T,typename std::remove_reference<decltype(func(std::declval<T>()).value())>::type>;

    /// Returns an iterator that will return only the first `count`
    /// elements of the underlying iterator
    fn::take<Derived,T> take(size_t count);

    /// Returns an iterator that will skip the first `count` elements
    /// of the underlying iterator
    fn::skip<Derived,T> skip(size_t count);

    /// Maps the underlying iterator to a different type through
    /// the predicate passed as parameter which receives an argument
    /// of the type of the current iterator and returns the mapped type
    template<typename F>
    auto flat_map(F func) -> fn::flat_map<Derived,T,decltype(func(std::declval<T>())),typename std::remove_reference<decltype(*func(std::declval<T>()).it())>::type>;

    /// Joins 2 iterators into a new iterator that returns a zipitem
    /// which will contain references to the elements in each of the
    /// underlying iterators. The end of the zip iterator happens at the
    /// end of any of the underlying iterators.
    template<typename OtherIT>
	fn::zip<Derived,T,OtherIT,typename OtherIT::type_name> zip(const OtherIT & other);

	/// Removes contiguous duplicated elements
	fn::dedup<Derived,T> dedup();

    bool contains(const T & t){
        for(auto & e: static_cast<Derived&>(*this)){
            if(t==e){
                return true;
            }
        }
        return false;
    }

    std::vector<T> to_vector(){
        return std::vector<T>(static_cast<const Derived&>(*this).begin(), static_cast<const Derived&>(*this).end());
    }
};
class none_t;

template<class T>
class option{
	T value_;
	bool is_;
	friend class none_t;
	template<class U>
	friend option<U> some(U t);
	explicit option()
		:is_(false){}
	explicit option(T t)
		:value_(t)
		,is_(true){}

public:
	bool is_some() const{
		return is_;
	}

	bool is_none() const{
		return !is_;
	}

	const T & value() const{
		return value_;
	}

	T & value(){
		return value_;
	}

	explicit operator bool() const{
		return is_some();
	}

	bool operator==(const none_t &) const{
		return is_none();
	}

	bool operator!=(const option<T> & other){
		return other.value_ != value_ && other.is_ != is_;
	}
};


class none_t{
public:
	template<class T>
	operator option<T>() const{
		return option<T>();
	}
};

const none_t none = none_t();

template<class T>
option<T> some(T t){
	return option<T>(t);
}

template<typename IT, typename T>
class advance: public base_iterator<T,advance<IT,T>,IT>{
    IT begin_, end_, it_;
    advance(const IT & begin, const IT & end, const IT & it)
    :begin_(begin)
    ,end_(end)
    ,it_(it){
    }

public:
    typedef IT BaseIT;

    inline advance(const IT & begin, const IT & end)
    :begin_(begin)
    ,end_(end)
    ,it_(begin){
    }

    inline const IT & it() const{
        return it_;
    }

    inline IT & it(){
        return it_;
    }

    inline advance begin() const{
        return advance(begin_, end_, begin_);
    }

    inline advance end() const{
        return advance(begin_, end_, end_);
    }

    inline T & operator*(){
        return *it_;
    }

    inline const T & operator*() const{
        return *it_;
    }

    inline advance & operator++(){
        if(it_!=end_){
            ++it_;
        }
        return *this;
    }

    inline advance operator+(size_t count) const{
        return advance(begin_, end_, it_ + count);
    }

    inline advance & operator+=(size_t count){
        it_ += count;
        return *this;
    }
};

template<typename IT, typename T>
class filter: public base_iterator<T,filter<IT,T>,typename IT::BaseIT>{
    IT it_;
    std::function<bool(const T&)> func;
public:
    typedef typename IT::BaseIT BaseIT;

    inline filter(const IT & it, std::function<bool(const T&)> func)
    :it_(it)
    ,func(func){
    }

    inline filter begin() const{
        auto begin = it_.begin();
        while(begin!=begin.end() && !func(*begin)){
            ++begin;
        }
        return filter(begin, func);
    }

    inline filter end() const{
        return filter(it_.end(), func);
    }

    inline T & operator*(){
        return *it_;
    }

    inline const T & operator*() const{
        return *it_;
    }

    inline filter & operator++(){
        while(it_!=it_.end()){
            ++it_;
            if(func(*it_)){
                break;
            }
        }
        return *this;
    }

    inline filter operator+(size_t count) const{
        filter next(*this);
        next += count;
        return next;
    }

    inline filter & operator+=(size_t count){
        for(size_t i=0;i<count;i++){
            operator++();
        }
        return *this;
    }

    inline const typename IT::BaseIT & it() const{
        return it_.it();
    }

    inline typename IT::BaseIT & it(){
        return it_.it();
    }
};

template<typename IT, typename T, typename U>
class map: public base_iterator<U,map<IT,T,U>,typename IT::BaseIT>{
    IT it_;
    U next_;
    std::function<U(const T&)> func;
public:
    typedef typename IT::BaseIT BaseIT;

    inline map(const IT & it, std::function<U(const T&)> func)
    :it_(it)
    ,next_(func(*it))
    ,func(func){
    }

    inline map begin() const{
        return map(it_.begin(), func);
    }

    inline map end() const{
        return map(it_.end(), func);
    }

    inline U & operator*(){
        return next_;
    }

    inline const U & operator*() const{
        return next_;
    }

    inline map & operator++(){
        if(it_!=it_.end()){
            ++it_;
            next_ = func(*it_);
        }
        return *this;
    }

    inline map operator+(size_t count) const{
        return map(it_ + count, func);
    }

    inline map & operator+=(size_t count){
        it_ += count;
        return *this;
    }

    inline const typename IT::BaseIT & it() const{
        return it_.it();
    }

    inline typename IT::BaseIT & it(){
        return it_.it();
    }
};


template<typename IT, typename T, typename U>
class filter_map: public base_iterator<U,filter_map<IT,T,U>,typename IT::BaseIT>{
    IT it_;
    option<U> next_;
    std::function<option<U>(const T&)> func;
public:
    typedef typename IT::BaseIT BaseIT;

    inline filter_map(const IT & it, std::function<option<U>(const T&)> func)
    :it_(it)
    ,next_(none)
    ,func(func){
        while(it_!=it_.end() && !(next_ = func(*it_))){
            ++it_;
        }
    }

    inline filter_map begin() const{
        return filter_map(it_.begin(), func);
    }

    inline filter_map end() const{
        return filter_map(it_.end(), func);
    }

    inline U & operator*(){
        return next_.value();
    }

    inline const U & operator*() const{
        return next_.value();
    }

    inline filter_map & operator++(){
        while(it_!=it_.end()){
            ++it_;
            if((next_ = func(*it_))){
                break;
            }
        }
        return *this;
    }

    inline filter_map operator+(size_t count) const{
        filter_map next(*this);
        next += count;
        return next;
    }

    inline filter_map & operator+=(size_t count){
        for(size_t i=0;i<count;i++){
            operator++();
        }
        return *this;
    }

    inline const typename IT::BaseIT & it() const{
        return it_.it();
    }

    inline typename IT::BaseIT & it(){
        return it_.it();
    }
};

template<typename T>
struct enumeration{
private:
    size_t idx_;
    T* value_;
public:
    inline enumeration(size_t idx, T&v)
    :idx_(idx)
    ,value_(&v){

    }

    inline enumeration()
    :value_(nullptr){

    }

    inline size_t index() const{
        return idx_;
    }

    inline T & value(){
        assert(value_);
        return *value_;
    }

    inline const T & value() const{
        assert(value_);
        return *value_;
    }
};

template<typename IT, typename T>
class enumerate: public base_iterator<enumeration<T>,enumerate<IT,T>,typename IT::BaseIT>{
    IT it_;
    enumeration<T> next_;
    enumerate(const IT & it, enumeration<T> next)
    :it_(it)
    ,next_(next){}
public:
    typedef typename IT::BaseIT BaseIT;

    inline enumerate(const IT & it)
    :it_(it)
    ,next_(0,*it_){}

    inline enumeration<T> & operator*() {
        return next_;
    }

    inline const enumeration<T> & operator*() const{
        return next_;
    }

    inline enumerate & operator++(){
        if(it_!=it_.end()){
            ++it_;
            next_ = {next_.index() + 1, *it_};
        }
        return *this;
    }

    inline enumerate operator+(size_t count) const{
        enumerate next(*this);
        next += count;
        return next;
    }

    inline enumerate & operator+=(size_t count){
        it_ += count;
        next_ = {next_.index() + count, *it_};
        return *this;
    }

    inline enumerate<IT,T> begin() const{
        auto begin = it_.begin();
        return enumerate(begin);
    }

    inline enumerate<IT,T> end() const{
        return enumerate(it_.end(),enumeration<T>());
    }

    inline const typename IT::BaseIT & it() const{
        return it_.it();
    }

    inline typename IT::BaseIT & it(){
        return it_.it();
    }
};

template<typename IT, typename T>
class take: public base_iterator<T,take<IT,T>,typename IT::BaseIT>{
    IT it_, end_;
    size_t count;

public:
    typedef typename IT::BaseIT BaseIT;

    inline take(const IT & it, size_t count)
    :it_(it)
    ,end_(it+count)
    ,count(count){}

    inline take begin() const{
        return take(it_.begin(), count);
    }

    inline take end() const{
        return take(end_, 0);
    }

    inline T & operator*(){
        return *it_;
    }

    inline const T & operator*() const{
        return *it_;
    }

    inline take & operator++(){
        ++it_;
        return *this;
    }

    inline take operator+(size_t count) const{
        return take(it_ + count, this->count - count);
    }

    inline take & operator+=(size_t count){
        it_ += count;
        return *this;
    }

    inline const typename IT::BaseIT & it() const{
        return it_.it();
    }

    inline typename IT::BaseIT & it(){
        return it_.it();
    }
};

template<typename IT, typename T>
class skip: public base_iterator<T,skip<IT,T>,typename IT::BaseIT>{
    IT it_, begin_;

public:
    typedef typename IT::BaseIT BaseIT;

    inline skip(const IT & it, size_t count)
    :it_(it + count)
    ,begin_(it_){}

    inline skip begin() const{
        return skip(begin_, 0);
    }

    inline skip end() const{
        return skip(it_.end(), 0);
    }

    inline T & operator*(){
        return *it_;
    }

    inline const T & operator*() const{
        return *it_;
    }

    inline skip & operator++(){
        ++it_;
        return *this;
    }

    inline skip operator+(size_t count) const{
        return skip(it_ + count, 0);
    }

    inline skip & operator+=(size_t count){
        it_ += count;
        return *this;
    }

    inline const typename IT::BaseIT & it() const{
        return it_.it();
    }

    inline typename IT::BaseIT & it(){
        return it_.it();
    }
};

template<typename IT, typename T, typename OtherIT, typename U>
class flat_map: public base_iterator<U,flat_map<IT,T,OtherIT,U>,typename IT::BaseIT>{
    IT it_;
    OtherIT next_;
    std::function<OtherIT(T&)> func;
public:
    typedef typename IT::BaseIT BaseIT;

    inline flat_map(const IT & it, std::function<OtherIT(T&)> func)
    :it_(it)
    ,next_(func(*it_))
    ,func(func){
    }

    inline flat_map begin() const{
        return flat_map(it_.begin(), func);
    }

    inline flat_map end() const{
        return flat_map(it_.end(), func);
    }

    inline U & operator*(){
        return *next_;
    }

    inline const U & operator*() const{
        return *next_;
    }

    inline flat_map & operator++(){
        if(next_!=next_.end()){
            ++next_;
        }
        if(next_==next_.end() && it_!=it_.end()){
            ++it_;
            next_ = func(*it_);
        }
        return *this;
    }

    inline flat_map operator+(size_t count) const{
        flat_map next(*this);
        next += count;
        return next;
    }

    inline flat_map & operator+=(size_t count){
        for(size_t i=0;i<count;i++){
            operator++();
        }
        return *this;
    }

    inline const typename IT::BaseIT & it() const{
        return it_.it();
    }

    inline typename IT::BaseIT & it(){
        return it_.it();
    }
};



template<typename IT, typename T>
class dedup: public base_iterator<T,dedup<IT,T>,typename IT::BaseIT>{
		IT it_;
	option<T> prev_;

	inline dedup(const IT & it, T prev)
	:it_(it)
	,prev_(some(prev)){}

public:
	typedef typename IT::BaseIT BaseIT;

	inline dedup(const IT & it)
	:it_(it)
	,prev_(none){}

	inline dedup begin() const{
		return dedup(it_.begin());
	}

	inline dedup end() const{
		return dedup(it_.end());
	}

	inline T & operator*(){
		return *it_;
	}

	inline const T & operator*() const{
		return *it_;
	}

	inline dedup & operator++(){
		do{
			++it_;
		}while(prev_.is_some() && *it_==prev_.value());
		prev_ = some(*it_);
		return *this;
	}

	inline dedup operator+(size_t count) const{
		auto ret = dedup(it_, prev_.value());
		ret += count;
		return ret;
	}

	inline dedup & operator+=(size_t count){
		for(size_t i=0;i<count;++i,++it_){}
		return *this;
	}

	inline const typename IT::BaseIT & it() const{
		return it_.it();
	}

	inline typename IT::BaseIT & it(){
		return it_.it();
	}
};

template<typename T>
class range_: public base_iterator<T,range_<T>,T>{
    T begin_, end_;
    T current;
    T step;
public:
    typedef T BaseIT;

    inline range_(T start, T end)
    :begin_(start)
    ,end_(end)
    ,current(start)
    ,step(1){

    }

    inline range_(T start, T end, T step)
    :begin_(start)
    ,end_(end)
    ,current(start)
    ,step(step){}

    inline range_ begin() const{
        return range_(begin_, end_, step);
    }

    inline range_ end() const{
        return range_(end_, end_, step);
    }

    inline T & operator*(){
        return current;
    }

    inline const T & operator*() const{
        return current;
    }

    inline range_ & operator++(){
        current += step;
        if((step>0 && current>end_) ||
            (step<0 && current<end_) ||
            abs(end_ - current) < std::numeric_limits<T>::epsilon()) current = end_;
        return *this;
    }

    inline range_ operator+(size_t count) const{
        auto next = *this;
        next += count;
        return next;
    }

    inline range_ & operator+=(size_t count){
        current += count;
        if((step>0 && current>end_) ||
            (step<0 && current<end_) ||
            abs(end_ - current) < std::numeric_limits<T>::epsilon()) current = end_;
        return *this;
    }

    inline const T & it() const{
        return current;
    }

    inline T & it(){
        return current;
    }
};

template<typename T>
range_<T> range(T start, T end){
    return range_<T>(start, end);
}

template<typename T>
range_<T> range(T start, T end, T step){
    return range_<T>(start, end, step);
}

template<typename T, typename U>
struct zipitem{
private:
    T * first_;
    U * second_;
public:
    T & first(){ return *first_;}
    U & second(){ return *second_;}
    zipitem(T & t, U & u)
    :first_(&t)
    ,second_(&u){}
};

template<typename IT, typename T, typename OtherIT, typename U>
class zip: public base_iterator<zipitem<T,U>,zip<IT,T,OtherIT,U>,typename IT::BaseIT>{
    IT it_;
    OtherIT other_;
    zipitem<T,U> next;
public:
    typedef typename IT::BaseIT BaseIT;

    inline zip(const IT & it, const OtherIT & other)
    :it_(it)
    ,other_(other)
    ,next(*it_,*other_){
    }

    inline zip begin() const{
        return zip(it_.begin(), other_.begin());
    }

    inline zip end() const{
        return zip(it_.end(), other_.end());
    }

    inline zipitem<T,U> & operator*(){
        return next;
    }

    inline const zipitem<T,U> & operator*() const{
        return next;
    }

    inline zip & operator++(){
        ++it_;
        ++other_;
        if(other_ == other_.end()) it_ = it_.end();
        next = zipitem<T,U>(*it_, *other_);
        return *this;
    }

    inline zip operator+(size_t count) const{
        zip next(*this);
        next += count;
        return next;
    }

    inline zip & operator+=(size_t count){
        it_ += count;
        other_ += count;
        if(other_ == other_.end()) it_ = it_.end();
        next = zipitem<T,U>(*it_, *other_);
        return *this;
    }

    inline const typename IT::BaseIT & it() const{
        return it_.it();
    }

    inline typename IT::BaseIT & it(){
        return it_.it();
    }
};

template<typename T, class Derived, class BaseIT>
fn::filter<Derived,T> base_iterator<T,Derived,BaseIT>::filter(std::function<bool(const T&)> func){
    return fn::filter<Derived,T>(static_cast<Derived&>(*this),func);
}

template<typename T, class Derived, class BaseIT>
template<typename F>
auto base_iterator<T,Derived,BaseIT>::map(F func) -> fn::map<Derived,T,decltype(func(std::declval<T>()))>{
    return fn::map<Derived,T,decltype(func(std::declval<T>()))>(static_cast<Derived&>(*this),func);
}

template<typename T, class Derived, class BaseIT>
template<typename F>
auto base_iterator<T,Derived,BaseIT>::filter_map(F func) -> fn::filter_map<Derived,T,typename std::remove_reference<decltype(func(std::declval<T>()).value())>::type>{
    return fn::filter_map<Derived,T,typename std::remove_reference<decltype(func(std::declval<T>()).value())>::type>(static_cast<Derived&>(*this),func);
}

template<typename T, class Derived, class BaseIT>
fn::enumerate<Derived,T> base_iterator<T,Derived,BaseIT>::enumerate(){
    return fn::enumerate<Derived,T>(static_cast<Derived&>(*this));
}

template<typename T, class Derived, class BaseIT>
fn::take<Derived,T> base_iterator<T,Derived,BaseIT>::take(size_t count){
    return fn::take<Derived,T>(static_cast<Derived&>(*this), count);
}

template<typename T, class Derived, class BaseIT>
fn::skip<Derived,T> base_iterator<T,Derived,BaseIT>::skip(size_t count){
    return fn::skip<Derived,T>(static_cast<Derived&>(*this), count);
}

template<typename T, class Derived, class BaseIT>
template<typename F>
auto base_iterator<T,Derived,BaseIT>::flat_map(F func) -> fn::flat_map<Derived,T,decltype(func(std::declval<T>())),typename std::remove_reference<decltype(*func(std::declval<T>()).it())>::type>{
    return fn::flat_map<Derived,T,decltype(func(std::declval<T>())),typename std::remove_reference<decltype(*func(std::declval<T>()).it())>::type>(static_cast<Derived&>(*this),func);
}

template<typename T, class Derived, class BaseIT>
template<typename OtherIT>
zip<Derived,T,OtherIT,typename OtherIT::type_name> base_iterator<T,Derived,BaseIT>::zip(const OtherIT & other){
    return fn::zip<Derived,T,OtherIT,typename OtherIT::type_name>(static_cast<Derived&>(*this), other);
}

template<typename T, class Derived, class BaseIT>
fn::dedup<Derived,T> base_iterator<T,Derived,BaseIT>::dedup(){
	return fn::dedup<Derived,T>(static_cast<Derived&>(*this));
}

/*template<typename T>
fn::advance<typename std::vector<T>::iterator,T> iter(std::vector<T> & vec){
    return fn::advance<typename std::vector<T>::iterator, T>(vec.begin(), vec.end());
}

template<typename IT,typename T>
fn::advance<IT,T> iter(const IT & begin, const IT & end){
    return fn::advance<IT, T>(begin,end);
}*/

template<typename Collection>
auto iter(Collection & c) -> fn::advance<decltype(c.begin()),typename std::remove_reference<decltype(*c.begin())>::type>{
    return fn::advance<decltype(c.begin()), typename std::remove_reference<decltype(*c.begin())>::type>(c.begin(),c.end());
}

}
