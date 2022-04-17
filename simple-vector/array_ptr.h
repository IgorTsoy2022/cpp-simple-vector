#pragma once

#include <cstddef>
#include <utility>

template <typename Type>
class ArrayPtr {
public:
    // �������������� ArrayPtr ������� ����������
    ArrayPtr() = default;

    // ������ � ���� ������ �� size ��������� ���� Type.
    // ���� size == 0, ���� raw_ptr_ ������ ���� ����� nullptr
    explicit ArrayPtr(size_t size) {
        if (size > 0) {
            raw_ptr_ = new Type[size]{};
            capacity_ = size;
        }
    }

    // ����������� �� ������ ���������, ��������� ����� 
    // ������� � ���� ���� nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }

    // ��������� �����������
    ArrayPtr(const ArrayPtr&) = delete;

    // ��������� ������������
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr(ArrayPtr&& other) {
        *this = std::move(other);
    }

    ArrayPtr& operator=(ArrayPtr&& rhs) {
        if (this != &rhs) {
/*
            if (raw_ptr_ != nullptr) {
                delete[] raw_ptr_;
                raw_ptr_ = nullptr;
                capacity_ = 0;
            }
*/
            swap(rhs);
        }
        return *this;
    }
/*
    size_t GetCapacity() const noexcept {
        return capacity_;
    }
*/
    void Delete() {
        delete[] raw_ptr_;
        raw_ptr_ = nullptr;
    }

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    // ���������� ��������� �������� � ������, ����������
    // �������� ������ �������
    // ����� ������ ������ ��������� �� ������ ������ ����������
    [[nodiscard]] Type* Release() noexcept {
        Type* tmp_ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return tmp_ptr;
    }

    // ���������� ������ �� ������� ������� � �������� index
    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    // ���������� ����������� ������ �� ������� ������� �
    // �������� index
    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    // ���������� true, ���� ��������� ���������, � false
    // � ��������� ������
    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    // ���������� �������� ������ ���������, ���������
    // ����� ������ �������
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // ������������ ��������� ��������� �� ������ �
    // �������� other
    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
        std::swap(capacity_, other.capacity_);
    }

private:
    Type* raw_ptr_ = nullptr;
    size_t capacity_ = 0;
};