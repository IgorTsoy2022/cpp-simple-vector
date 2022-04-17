#pragma once

#include "array_ptr.h"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iostream>

using namespace std::string_literals;

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t value)
        : value_(value)
    {}

    size_t GetValue() const {
        return value_;
    }

private:
    size_t value_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // ������ ������ �� size ���������, ������������������
    // ��������� �� ���������
    explicit SimpleVector(size_t size)
        : capacity_(size), size_(size), data_(size)
    {}

    // ������ ������ �� size ���������, ������������������
    // ��������� value
    SimpleVector(size_t size, const Type& value)
        : capacity_(size), size_(size), data_(size)
    {
        std::fill(begin(), end(), value);
    }

    // ������ ������ �� std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        Assign(init.begin(), init.end());
    }

    SimpleVector(const SimpleVector& other) {
        *this = other;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            Assign(rhs.begin(), rhs.end());
        }
        return *this;
    }

    SimpleVector(SimpleVector&& other) {
        *this = std::move(other);
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            capacity_ = std::exchange(rhs.capacity_, 0);
            size_ = std::exchange(rhs.size_, 0);
            data_.Delete();
            data_.swap(rhs.data_);
        }
        return *this;
    }

    SimpleVector(const ReserveProxyObj& Rsrv) {
        Reserve(Rsrv.GetValue());
    }

    // ���������� ����������� �������
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // ���������� ���������� ��������� � �������
    size_t GetSize() const noexcept {
        return size_;
    }

    // ��������, ������ �� ������
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // ���������� ������ �� ������� � �������� index
    Type& operator[](size_t index) noexcept {
        return data_[index];
    }

    // ���������� ����������� ������ �� ������� � ��������
    // index
    const Type& operator[](size_t index) const noexcept {
        return data_[index];
    }

    // ���������� ����������� ������ �� ������� � ��������
    // index
    // ����������� ���������� std::out_of_range, ����
    // index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Out of range!");
        }
        return data_[index];
    }

    // ���������� ����������� ������ �� ������� � ��������
    // index
    // ����������� ���������� std::out_of_range, ����
    // index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Out of range!");
        }
        return data_[index];
    }

    // �������� ������ �������, �� ������� ��� �����������
    void Clear() noexcept {
        size_ = 0;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ResizeCapacity(new_capacity);
        }
    }

    // �������� ������ �������.
    // ��� ���������� ������� ����� �������� �������� ��������
    // �� ��������� ��� ���� Type
    void Resize(size_t new_size) {
         if (new_size <= size_) {
            size_ = new_size;
            return;
        }
        if (new_size <= capacity_) {
            for (auto it = &data_[size_];
                      it != &data_[new_size]; ++it) {
                *it = std::move(Type{});                  
            }
    //        std::fill(&data_[size_], &data_[new_size],
    //                  std::move(Type{}));
            size_ = new_size;
            return;
        }
        ResizeCapacity(std::max(new_size, 2 * capacity_));
        size_ = new_size;
    }

    // ��������� ������� � ����� �������
    // ��� �������� ����� ����������� ����� ����������� �������
    void PushBack(const Type& value) {
        if (capacity_ == size_) {
            ResizeCapacity(size_ == 0 ? 1 : size_ * 2);
        }
        data_[size_++] = value;
    }

    void PushBack(Type&& value) {
        if (capacity_ == size_) {
            ResizeCapacity(size_ == 0 ? 1 : size_ * 2);
        }
        data_[size_++] = std::move(value);
    }

    // "�������" ��������� ������� �������. ������ �� ������
    // ���� ������
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    // ��������� �������� value � ������� pos.
    // ���������� �������� �� ����������� ��������
    // ���� ����� �������� �������� ������ ��� ��������
    // ���������, ����������� ������� ������ �����������
    // �����, � ��� ������� ������������ 0 ����� ������ 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        Iterator it = const_cast<Iterator>(pos);
        if (capacity_ > size_) {
            std::move_backward(std::make_move_iterator(it),
                               std::make_move_iterator(end()),
                               &data_[size_ + 1]);
            *it = std::move(value);
            ++size_;
            return it;
        }
        if (capacity_ == 0) {
            ArrayPtr<Type> tmp_data(1);
            tmp_data[0] = std::move(value);
            data_ = std::move(tmp_data);
            capacity_ = 1;
            size_ = 1;
            return &data_[0];
        }
        size_t new_capacity = size_ * 2;
        size_t insert_index = std::distance(begin(), it);
        ArrayPtr<Type> tmp_data(new_capacity);
        std::move(std::make_move_iterator(begin()),
                  std::make_move_iterator(it),
                  &tmp_data[0]);
        tmp_data[insert_index] = std::move(value);
        std::move(std::make_move_iterator(it),
                  std::make_move_iterator(end()),
                  &tmp_data[insert_index + 1]);
        data_ = std::move(tmp_data);
        capacity_ = new_capacity;
        ++size_;
        return &data_[insert_index];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        Iterator it = const_cast<Iterator>(pos);
        if (capacity_ > size_) {
            std::move_backward(std::make_move_iterator(it),
                               std::make_move_iterator(end()),
                               &data_[size_ + 1]);
            *it = std::move(value);
            ++size_;
            return it;
        }
        if (capacity_ == 0) {
            ArrayPtr<Type> tmp_data(1);
            tmp_data[0] = std::move(value);
            data_ = std::move(tmp_data);
            capacity_ = 1;
            size_ = 1;
            return &data_[0];
        }
        size_t new_capacity = size_ * 2;
        size_t insert_index = std::distance(begin(), it);
        ArrayPtr<Type> tmp_data(new_capacity);
        std::move(std::make_move_iterator(begin()),
                  std::make_move_iterator(it),
                  &tmp_data[0]);
        tmp_data[insert_index] = std::move(value);
        std::move(std::make_move_iterator(it),
                  std::make_move_iterator(end()),
                  &tmp_data[insert_index + 1]);
        data_ = std::move(tmp_data);
        capacity_ = new_capacity;
        ++size_;
        return &data_[insert_index];
    }

    // ������� ������� ������� � ��������� �������
    Iterator Erase(ConstIterator pos) {
        Iterator it = const_cast<Iterator>(pos);
        size_t index = std::distance(begin(), it);
        if (size_ > index + 1) {
            for (auto i = index; i < size_ - 1; ++i) {
                data_[i] = std::move(data_[i + 1]);
            }
        }
        --size_;
        return it;
    }

    // ���������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����)
    // nullptr
    Iterator begin() noexcept {
        return data_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����)
    // nullptr
    Iterator end() noexcept {
        return &data_[size_];
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����)
    // nullptr
    ConstIterator begin() const noexcept {
        return data_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����)
    // nullptr
    ConstIterator end() const noexcept {
        return &data_[size_];
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����)
    // nullptr
    ConstIterator cbegin() const noexcept {
        return const_cast<Type*>(data_.Get());
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����)
    // nullptr
    ConstIterator cend() const noexcept {
        return &data_[size_];
    }

    void swap(SimpleVector& other) noexcept {
        data_.swap(other.data_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

    ~SimpleVector()
    {}

private:
    size_t capacity_ = 0;
    size_t size_ = 0;
    ArrayPtr<Type> data_;

    template <typename InputIterator>
    void Assign(InputIterator from, InputIterator to) {
        size_t size = std::distance(from, to);
        SimpleVector<Type> tmp(size);
        if (size > 0) {
            std::move(std::make_move_iterator(from),
                      std::make_move_iterator(to), tmp.begin());
        }
        swap(tmp);
    }

    void ResizeCapacity(size_t new_capacity) {
        ArrayPtr<Type> tmp_data(new_capacity);
        std::move(std::make_move_iterator(begin()),
                  std::make_move_iterator(end()), &tmp_data[0]);
        data_ = std::move(tmp_data);
        capacity_ = new_capacity;
    }

};

template <typename Type>
void swap(SimpleVector<Type>& lhs,
          SimpleVector<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename Type>
bool operator==(const SimpleVector<Type>& lhs,
                const SimpleVector<Type>& rhs) {
    return (&lhs == &rhs)  // ����������� ��������� ������
                           // � �����
        || (lhs.GetSize() == rhs.GetSize()
            && std::equal(lhs.begin(), lhs.end(),
                          rhs.begin())); // ����� �������
                                         // ����������
}

template <typename Type>
bool operator!=(const SimpleVector<Type>& lhs,
                const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);  // ����� ������� ����������
}

template <typename Type>
bool operator<(const SimpleVector<Type>& lhs,
               const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
    // ����� ������� ����������
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs,
                const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);  // ����� ������� ����������
}

template <typename Type>
bool operator>(const SimpleVector<Type>& lhs,
               const SimpleVector<Type>& rhs) {
    return (rhs < lhs);  // ����� ������� ����������
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs,
                const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);  // ����� ������� ����������
}