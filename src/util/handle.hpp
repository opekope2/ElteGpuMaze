template <typename H, auto DeleteFn>
class Handle {
private:
    H handle;

    void destroy() {
        if (handle)
            DeleteFn(handle);
        handle = 0;
    }

public:
    Handle() = delete;
    Handle(H handle) : handle(handle) {}

    ~Handle() { destroy(); }

    Handle(const Handle &) = delete;
    Handle &operator=(const Handle &) = delete;

    Handle(Handle &&other) noexcept : handle(other.handle) { other.handle = 0; }
    Handle &operator=(Handle &&other) noexcept {
        if (this != &other) {
            destroy();
            handle = other.handle;
            other.handle = 0;
        }
        return *this;
    }

    inline H get() const { return handle; }
    inline operator H() const { return handle; }
};
