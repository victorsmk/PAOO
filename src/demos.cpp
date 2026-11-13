#include <l1/demos.hpp>
#include <l1/bad_examples.hpp>
#include <l1/buffer.hpp>
#include <l1/tracker.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>

namespace paoo
{

    namespace
    {

        void title(const std::string &text)
        {
            std::cout << std::endl
                      << "=== " << text << " ===" << std::endl;
        }

        // A member that reports which of its special functions get called
        class Noisy
        {
        public:
            Noisy() : tag_("default") { std::cout << "    Noisy()" << std::endl; }
            explicit Noisy(const std::string &tag) : tag_(tag)
            {
                std::cout << "    Noisy(\"" << tag << "\")" << std::endl;
            }
            Noisy(const Noisy &) = default;
            Noisy &operator=(const Noisy &other)
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
            Order() : second_("second"), first_("first") {} // -Wreorder warning is expected
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

        // Compiler writes: default ctor, copy ctor, copy assignment,
        // move ctor, move assignment, destructor
        class Empty
        {
        };

        // Copying explicitly forbidden
        class Uncopyable
        {
        public:
            Uncopyable() = default;
            Uncopyable(const Uncopyable &) = delete;
            Uncopyable &operator=(const Uncopyable &) = delete;
        };

        // derived class that copies ALL parts (base part + its own member)
        class LabeledBuffer : public Buffer
        {
        public:
            LabeledBuffer(const std::string &label, std::size_t size, int fill)
                : Buffer(label, size, fill), label_(label) {}

            LabeledBuffer(const LabeledBuffer &other)
                : Buffer(other) // base part
                  ,
                  label_(other.label_) // derived part
            {
            }

            LabeledBuffer &operator=(const LabeledBuffer &other)
            {
                Buffer::operator=(other); // base part
                label_ = other.label_;    // derived part
                return *this;
            }

            const std::string &label() const { return label_; }

        private:
            std::string label_;
        };

        // Same class, but the base part is forgotten
        class BrokenLabeledBuffer : public Buffer
        {
        public:
            BrokenLabeledBuffer(const std::string &label, std::size_t size, int fill)
                : Buffer(label, size, fill), label_(label) {}

            BrokenLabeledBuffer(const BrokenLabeledBuffer &other)
                : Buffer("?", 0) // base part NOT copied
                  ,
                  label_(other.label_)
            {
            }

            BrokenLabeledBuffer &operator=(const BrokenLabeledBuffer &other)
            {
                label_ = other.label_; // base part NOT assigned
                return *this;
            }

            const std::string &label() const { return label_; }

        private:
            std::string label_;
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
            b.set(10, 1); // the only way to write goes through a checked function
        }
        catch (const std::out_of_range &)
        {
            std::cout << "set(10, ...) rejected: index out of range" << std::endl;
        }
    }

    // 3. Member initialization in the constructor
    void demoInitializerList()
    {
        title("3. Initializer list");

        std::cout << "Assignment in the constructor body (2 steps + temporary):" << std::endl;
        {
            AssignInBody x;
        }

        std::cout << "Initializer list (1 step):" << std::endl;
        {
            InitInList x;
        }

        std::cout << "Order follows the declaration (first_, second_), not the list:" << std::endl;
        {
            Order x;
        }

        WithConst c(5);
        std::cout << "const member initialized only via the list: id = " << c.id() << std::endl;
    }

    // 4. Releasing the heap in the destructor
    void demoDestructor()
    {
        title("4. Destructor");
        resetCounters();

        {
            LeakyBuffer leaky(1000);
        }
        std::cout << "After LeakyBuffer went out of scope, live heap blocks = "
                  << liveBlocks() << " (expected 0 -> memory leak)" << std::endl;

        {
            Buffer ok("ok", 1000);
        }
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
        ShallowBuffer *a = new ShallowBuffer(3);
        ShallowBuffer *b = new ShallowBuffer(*a); // implicit member-wise copy
        std::cout << "a.data = " << static_cast<void *>(a->data())
                  << ", b.data = " << static_cast<void *>(b->data())
                  << "  -> same address, one block shared" << std::endl;

        b->data()[0] = 99;
        std::cout << "b[0] = 99  =>  a[0] = " << a->data()[0]
                  << " (changing the copy changed the original)" << std::endl;

        delete a; // frees the shared block; b->data() now dangles
        std::cout << "'a' destroyed: b now points to freed memory." << std::endl;

        if (crash)
        {
            std::cout << "Destroying 'b' -> second delete[] of the same block..." << std::endl;
            delete b; // double free: the program aborts here
        }
        else
        {
            std::cout << "('b' is deliberately not deleted; run with the argument "
                         "'double-free' to see the crash)"
                      << std::endl;
        }

        std::cout << std::endl
                  << "-- With our copy constructor (Buffer) --" << std::endl;
        resetCounters();
        {
            Buffer x("x", 3, 1);
            Buffer y(x);
            std::cout << "x.raw = " << static_cast<const void *>(x.raw())
                      << ", y.raw = " << static_cast<const void *>(y.raw())
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
        const int *before = src.raw();
        Buffer dst(std::move(src));
        std::cout << "address before move = " << static_cast<const void *>(before)
                  << ", dst.raw = " << static_cast<const void *>(dst.raw())
                  << "  -> same block, no allocation" << std::endl;
        std::cout << "src is now empty: size = " << src.size()
                  << ", raw = " << static_cast<const void *>(src.raw()) << std::endl;
        std::cout << "live heap blocks = " << liveBlocks() << " (still one)" << std::endl;
    }

    // what the compiler writes, and how to forbid it
    void demoCompilerGeneratedFunctions()
    {
        title("Compiler-generated functions");

        // an empty class gets everything for free
        static_assert(std::is_default_constructible<Empty>::value, "");
        static_assert(std::is_copy_constructible<Empty>::value, "");
        static_assert(std::is_copy_assignable<Empty>::value, "");
        static_assert(std::is_nothrow_move_constructible<Empty>::value, "");

        // Declaring any constructor removes the implicit default constructor
        static_assert(!std::is_default_constructible<Buffer>::value, "");

        // Declaring a copy ctor/destructor means no implicit move ctor is written
        static_assert(std::is_nothrow_move_constructible<Buffer>::value, "");
        static_assert(!std::is_nothrow_move_constructible<CopyOnlyBuffer>::value, "");

        // The compiler also wrote a copy ASSIGNMENT for ShallowBuffer
        static_assert(std::is_copy_assignable<ShallowBuffer>::value, "");

        std::cout << "Compiler-generated members checked at compile time (static_assert)."
                  << std::endl;

        resetCounters();
        ShallowBuffer *a = new ShallowBuffer(3);
        ShallowBuffer *c = new ShallowBuffer(3);
        *c = *a; // generated operator=: copies the pointer, c's old block is lost
        std::cout << "After *c = *a: a.data = " << static_cast<void *>(a->data())
                  << ", c.data = " << static_cast<void *>(c->data())
                  << " (shared)" << std::endl;
        delete a; // frees the shared block; 'c' is deliberately not deleted (double free)
        std::cout << "live heap blocks = " << liveBlocks()
                  << " (c's original block is leaked)" << std::endl;

        // explicitly forbidden operations do not compile
        static_assert(!std::is_copy_assignable<Uncopyable>::value, "operator= is deleted");
        static_assert(!std::is_copy_constructible<Uncopyable>::value, "copy ctor is deleted");

        // Uncopyable u1; Uncopyable u2(u1);   // error: use of deleted function

        std::cout << "Forbidden operations rejected at compile time "
                     "(see the commented lines in demos.cpp)."
                  << std::endl;
    }

    // Items 10, 11 and 12
    void demoAssignment(bool unsafe)
    {
        title("Assignment operators (Items 10, 11, 12)");

        // Item 10: operator= returns a reference to *this, so chaining works
        static_assert(std::is_same<decltype(std::declval<Buffer &>() = std::declval<const Buffer &>()),
                                   Buffer &>::value,
                      "operator= must return Buffer&");
        Buffer a("a", 2, 1), b("b", 2, 2), c("c", 2, 3);
        a = b = c;
        std::cout << "a = b = c  =>  a[0] = " << a.at(0) << ", b[0] = " << b.at(0)
                  << " (both 3)" << std::endl;

        // Item 11: self-assignment
        Buffer s("s", 4, 7);
        Buffer &alias = s;
        const int *before = s.raw();
        s = alias;
        s = std::move(alias);
        std::cout << "Buffer after self-assignment (copy and move): s[0] = " << s.at(0)
                  << ", same block = " << (before == s.raw()) << std::endl;

        if (unsafe)
        {
            NaiveAssignBuffer n(4, 7);
            NaiveAssignBuffer &nAlias = n;
            n = nAlias;
            std::cout << "NaiveAssignBuffer after self-assignment: n[0] = " << n.at(0)
                      << " (was 7; undefined behavior)" << std::endl;
        }
        else
        {
            std::cout << "(NaiveAssignBuffer self-assignment skipped: undefined behavior; "
                         "run with the argument 'self-assign')"
                      << std::endl;
        }

        // Item 12: copy all parts
        LabeledBuffer ok1("ok1", 3, 5), ok2("ok2", 3, 9);
        ok2 = ok1;
        LabeledBuffer ok3(ok1);
        std::cout << "LabeledBuffer assignment: label = " << ok2.label()
                  << ", [0] = " << ok2.at(0) << " (5); copy ctor size = " << ok3.size()
                  << " (3)" << std::endl;

        BrokenLabeledBuffer bad1("bad1", 3, 5), bad2("bad2", 3, 9);
        bad2 = bad1;
        BrokenLabeledBuffer bad3(bad1);
        std::cout << "BrokenLabeledBuffer assignment: label = " << bad2.label()
                  << " (copied), [0] = " << bad2.at(0) << " (still 9: base part not copied); "
                  << "copy ctor size = " << bad3.size() << " (should be 3)" << std::endl;
    }

} // namespace paoo