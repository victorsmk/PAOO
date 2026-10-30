#include <l1/demos.hpp>
#include <l1/bad_examples.hpp>
#include <l1/buffer.hpp>
#include <l1/tracker.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace paoo {

namespace {

void title(const std::string& text)
{
    std::cout << std::endl << "=== " << text << " ===" << std::endl;
}

// A member that reports which of its special functions get called
class Noisy
{
public:
    Noisy() : tag_("default") { std::cout << "    Noisy()" << std::endl; }
    explicit Noisy(const std::string& tag) : tag_(tag)
    {
        std::cout << "    Noisy(\"" << tag << "\")" << std::endl;
    }
    Noisy(const Noisy&) = default;
    Noisy& operator=(const Noisy& other)
    {
        tag_ = other.tag_;
        std::cout << "    Noisy::operator=" << std::endl;
        return *this;
    }

private:
    std::string tag_;
};

// Member set in the constructor BODY: default-constructed first, then assigned
class AssignInBody
{
public:
    AssignInBody() { member_ = Noisy("a"); }
private:
    Noisy member_;
};

// Member set in the initializer list: constructed once, directly
class InitInList
{
public:
    InitInList() : member_("a") {}
private:
    Noisy member_;
};

// Members are initialized in DECLARATION order, not in the order of the list
class Order
{
public:
    Order() : second_("second"), first_("first") {}   // -Wreorder warning is expected
private:
    Noisy first_;
    Noisy second_;
};

// const members (and references) can ONLY be initialized in the list
class WithConst
{
public:
    explicit WithConst(int id) : id_(id) {}
    // explicit WithConst(int id) { id_ = id; }   // error: assignment of read-only member
    int id() const { return id_; }
private:
    const int id_;
};

} // namespace

// 2. Encapsulation
void demoEncapsulation()
{
    title("2. Encapsulation");

    Buffer b("b", 4, 7);
    b.set(1, 42);
    std::cout << "size = " << b.size() << ", b[1] = " << b.at(1) << std::endl;

    // b.size_ = 1000;      // error: 'size_' is private -> the invariant cannot be broken
    // b.data_[10] = 1;     // error: 'data_' is private

    try
    {
        b.set(10, 1);       // the only way to write goes through a checked function
    }
    catch (const std::out_of_range&)
    {
        std::cout << "set(10, ...) rejected: index out of range" << std::endl;
    }
}

// 3. Member initialization in the constructor
void demoInitializerList()
{
    title("3. Initializer list");

    std::cout << "Assignment in the constructor body (2 steps + temporary):" << std::endl;
    { AssignInBody x; }

    std::cout << "Initializer list (1 step):" << std::endl;
    { InitInList x; }

    std::cout << "Order follows the declaration (first_, second_), not the list:" << std::endl;
    { Order x; }

    WithConst c(5);
    std::cout << "const member initialized only via the list: id = " << c.id() << std::endl;
}

// 4. Releasing the heap in the destructor
void demoDestructor()
{
    title("4. Destructor");
    resetCounters();

    { LeakyBuffer leaky(1000); }
    std::cout << "After LeakyBuffer went out of scope, live heap blocks = "
              << liveBlocks() << " (expected 0 -> memory leak)" << std::endl;

    { Buffer ok("ok", 1000); }
    std::cout << "After Buffer went out of scope, live heap blocks = "
              << liveBlocks() << " (still 1: only the leaky one remains)" << std::endl;
}

// 5. Copy constructor
void demoCopyConstructor(bool crash)
{
    title("5. Copy constructor");
    resetCounters();

    std::cout << "-- Without a copy constructor (ShallowBuffer) --" << std::endl;
    // Allocated with new so we control exactly when each destructor runs
    ShallowBuffer* a = new ShallowBuffer(3);
    ShallowBuffer* b = new ShallowBuffer(*a);     // implicit member-wise copy
    std::cout << "a.data = " << static_cast<void*>(a->data())
              << ", b.data = " << static_cast<void*>(b->data())
              << "  -> same address, one block shared" << std::endl;

    b->data()[0] = 99;
    std::cout << "b[0] = 99  =>  a[0] = " << a->data()[0]
              << " (changing the copy changed the original)" << std::endl;

    delete a;   // frees the shared block; b->data() now dangles
    std::cout << "'a' destroyed: b now points to freed memory." << std::endl;

    if (crash)
    {
        std::cout << "Destroying 'b' -> second delete[] of the same block..." << std::endl;
        delete b;   // double free: the program aborts here
    }
    else
    {
        std::cout << "('b' is deliberately not deleted; run with the argument "
                     "'double-free' to see the crash)" << std::endl;
    }

    std::cout << std::endl << "-- With our copy constructor (Buffer) --" << std::endl;
    resetCounters();
    {
        Buffer x("x", 3, 1);
        Buffer y(x);
        std::cout << "x.raw = " << static_cast<const void*>(x.raw())
                  << ", y.raw = " << static_cast<const void*>(y.raw())
                  << "  -> different blocks" << std::endl;

        y.set(0, 99);
        std::cout << "y[0] = 99  =>  x[0] = " << x.at(0) << " (original untouched)" << std::endl;
        std::cout << "live heap blocks = " << liveBlocks() << " (one per object)" << std::endl;
    }
    std::cout << "after scope, live heap blocks = " << liveBlocks() << std::endl;
}

// 6. Move constructor
void demoMoveConstructor()
{
    title("6. Move constructor");

    const int count = 8;

    resetCounters();
    {
        std::vector<CopyOnlyBuffer> v;
        for (int i = 0; i < count; ++i)
            v.push_back(CopyOnlyBuffer(100000));
        std::cout << "CopyOnlyBuffer (no move ctor): " << copyCount()
                  << " deep copies of 100000 ints, " << moveCount() << " moves" << std::endl;
    }

    resetCounters();
    {
        std::vector<Buffer> v;
        for (int i = 0; i < count; ++i)
            v.push_back(Buffer("tmp", 100000));
        std::cout << "Buffer (with move ctor):       " << copyCount()
                  << " deep copies, " << moveCount() << " moves (pointer steals)" << std::endl;
    }

    resetCounters();
    Buffer src("src", 5, 3);
    const int* before = src.raw();
    Buffer dst(std::move(src));
    std::cout << "address before move = " << static_cast<const void*>(before)
              << ", dst.raw = " << static_cast<const void*>(dst.raw())
              << "  -> same block, no allocation" << std::endl;
    std::cout << "src is now empty: size = " << src.size()
              << ", raw = " << static_cast<const void*>(src.raw()) << std::endl;
    std::cout << "live heap blocks = " << liveBlocks() << " (still one)" << std::endl;
}

} // namespace paoo