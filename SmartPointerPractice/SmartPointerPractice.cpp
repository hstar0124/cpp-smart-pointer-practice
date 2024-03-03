#include <iostream>
#include <thread>
#include <vector>
#include <map>

using namespace std;


class RefCountable
{
public:
	RefCountable() { }
	virtual ~RefCountable() { }

	int GetRef() { return _refCount; }
	int AddRef() { return ++_refCount; }
	int ReleaseRef() 
	{
		int refCount = --_refCount;
		if (refCount == 0)
			delete this;

		return refCount;
	}

protected:
	atomic<int> _refCount = 1;
};

// 
template<typename T>
class TSharedPtr
{
public:
	TSharedPtr() {}
	TSharedPtr(T* ptr) { Set(ptr); }

	// 복사
	TSharedPtr(const TSharedPtr& other) { Set(other._ptr); }

	// 이동
	TSharedPtr(TSharedPtr&& other) { _ptr = other._ptr; other._ptr = nullptr; }

	// 상속 관계 복사
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs) { Set(static_cast<T*>(rhs._ptr)); }

	~TSharedPtr() { Release(); }

	TSharedPtr& operator=(const TSharedPtr& rhs)
	{
		if (_ptr != rhs._ptr)
		{
			Release();
			Set(rhs._ptr);
		}
		return *this;
	}

	TSharedPtr& operator=(TSharedPtr&& rhs)
	{
		Release();
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
		return *this;
	}

	bool		operator==(const TSharedPtr& rhs) const { return _ptr == rhs._ptr; }
	bool		operator==(T* ptr) const { return _ptr == ptr; }
	bool		operator!=(const TSharedPtr& rhs) const { return _ptr != rhs._ptr; }
	bool		operator!=(T* ptr) const { return _ptr != ptr; }
	bool		operator<(const TSharedPtr& rhs) const { reutrn _ptr < rhs._ptr; }
	T*			operator*() { return _ptr; }
	const T*	operator*() const { return _ptr; }
				operator T* () const { return _ptr; }
	T*			operator->() { return _ptr; }
	const T*	operator->() const { return _ptr; }

	bool IsNull() { return _ptr == nullptr; }


private:
	void Set(T* ptr)
	{
		_ptr = ptr;
		if (ptr)
			ptr->AddRef();
	}

	void Release()
	{
		if (_ptr != nullptr)
		{
			_ptr->ReleaseRef();
			_ptr = nullptr;
		}
	}


private:
	T* _ptr = nullptr;
};

class Knight : public RefCountable
{
};

map<int, Knight*> _knights;

void Test(Knight* knight)
{
	_knights[100] = knight;

	// 내가 Ref를 늘리기 전에 어디선가 ReleaseRef 가 호출한다면?
	// knight 포인터는 안전하지 않다.
	// 그래서 수동으로 RefCount를 관리하는건 매우 위험한 일이다.
	// 스마트포인터를 이용해 이를 해결할 수 있다.

	knight->AddRef();
}

using KnightRef = TSharedPtr<Knight>;

// 이 경우에는 이 곳에 복사되는 순간 RefCount가 1 증가 된 상태로 넘어오기 때문에
// 안전하다.
// 하지만 함수 안에서 잠시만 사용할 생각인데, 
// 복사비용이 부담된다면 레퍼런스를 붙여서 
// 레퍼런스 카운트를 늘리지 않고 사용할 수 있다.
void Test(KnightRef& knight)
{
	// TODO

	
}

class Archer : public enable_shared_from_this<Archer>
{
public:
	void Test()
	{
		// Move(this);		// 스마트포인터 사용시 내 자신을 어떻게 넘겨줘야하는가?
		// shared_ptr<Archer> a = shared_ptr<Archer>(this);
		// 위 방식으로하면 스마트포인터가 새로 생성이 되서 RefCount가 2개가 각각 1씩 관리가 되고
		// 둘 중에 하나라도 Release가 된다면 자폭하게 되어 다른 스마트포인토에서도 접근이 불가능하다.

		// 자신을 넘기기 위해서는 Class 에
		// public enable_shared_from_this<Archer> 을 명시하고
		// 아래 메소드를 호출하면 된다.
		Move(shared_from_this());
	}

	void Move(shared_ptr<Archer> a)
	{

	}
};


int main()
{
	//Knight* knight = new Knight();

	//thread t(Test, knight);

	//knight->ReleaseRef();

	//t.join();

	KnightRef knight(new Knight());

	Test(knight);

	shared_ptr<Archer> a(new Archer());
	a->Test();
}
