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


int main()
{
	//Knight* knight = new Knight();

	//thread t(Test, knight);

	//knight->ReleaseRef();

	//t.join();

	KnightRef knight(new Knight());

	Test(knight);

}
