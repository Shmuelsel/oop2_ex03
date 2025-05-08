#pragma once

#include <vector>
#include <iostream>


template <typename T>
class SquareMatrix
{
public:
	SquareMatrix(const SquareMatrix&) = default;
	SquareMatrix(SquareMatrix&&) = default;
	SquareMatrix& operator=(const SquareMatrix&) = default;
	SquareMatrix& operator=(SquareMatrix&&) = default;
	~SquareMatrix() = default;
	//SquareMatrix(const std::vector<std::vector<T>>& matrix);
	//SquareMatrix(std::vector<std::vector<T>>&& matrix);
	SquareMatrix(int size, const T& value);
	SquareMatrix(int size);
	int size() const
	{
		return m_size;
	};
	T& operator()(int i, int j);
	const T& operator()(int i, int j) const;
	SquareMatrix& operator+=(const SquareMatrix& rhs);
	SquareMatrix& operator-=(const SquareMatrix& rhs);
	//SquareMatrix& operator*=(const SquareMatrix& rhs);
	//SquareMatrix& operator*=(const T& scalar);
	SquareMatrix operator+(const SquareMatrix& rhs) const;
	SquareMatrix operator-(const SquareMatrix& rhs) const;
	//SquareMatrix operator*(const SquareMatrix& rhs) const;
	SquareMatrix operator*(const T& scalar) const;
	//bool operator==(const SquareMatrix& rhs) const;
	//bool operator!=(const SquareMatrix& rhs) const;
	SquareMatrix Transpose() const;
	//void print(std::ostream& ostr) const;
private:
	int m_size;
	std::vector<std::vector<T>> m_matrix;
};

template <typename T>
const T& SquareMatrix<T>::operator()(int i, int j) const
{
	return m_matrix[i][j];
}

template <typename T>
T& SquareMatrix<T>::operator()(int i, int j)
{
	return m_matrix[i][j];
}

inline std::ostream& operator<<(std::ostream& ostr, const SquareMatrix<int>& matrix)
{
	for (int i = 0; i < matrix.size(); ++i)
	{
		for (int j = 0; j < matrix.size(); ++j)
		{
			ostr << matrix(i, j) << ' ';
		}
		ostr << '\n';
	}
	return ostr;
}

inline std::istream& operator>>(std::istream& istr, SquareMatrix<int>& matrix)
{
	//istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	//if there is more input then trow an exception
	
	for (int i = 0; i < matrix.size(); ++i)
	{
		for (int j = 0; j < matrix.size(); ++j)
		{
			istr >> matrix(i, j);
			if (istr.fail())
			{
				istr.clear();
				istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::invalid_argument("Invalid input: expected an integer for matrix");
			}
			//chack if not bigger than 1000 or smaller than -1024
			if (matrix(i, j) > 1000 || matrix(i, j) < -1024)
			{
				istr.clear();
				istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				throw std::out_of_range("Matrix value is out of range");
			}
		}
	}
	return istr;
}

// Implementation must be in .h file for the compiler to see it and instantiate
// the relevant function
template <typename T>
SquareMatrix<T>::SquareMatrix(int size, const T& value)
	: m_size(size), m_matrix(size, std::vector<T>(size, value))
{
	for (int i = 0; i < size * size; ++i)
	{
		m_matrix[i / size][i % size] = value;
	}
}

template <typename T>
SquareMatrix<T>::SquareMatrix(int size)
	: m_size(size), m_matrix(size, std::vector<T>(size))
{
	for (int i = 0; i < size * size; ++i)
	{
		m_matrix[i / size][i % size] = i;
	}
}

template <typename T>
SquareMatrix<T> SquareMatrix<T>::operator+(const SquareMatrix& rhs) const
{
	SquareMatrix result(*this);
	return result += rhs;
}


template <typename T>
SquareMatrix<T> SquareMatrix<T>::operator-(const SquareMatrix& rhs) const
{
	SquareMatrix result(*this);
	return result -= rhs;
}

template <typename T>
SquareMatrix<T>& SquareMatrix<T>::operator+=(const SquareMatrix& rhs)
{
	for (int i = 0; i < m_size; ++i)
	{
		for (int j = 0; j < m_size; ++j)
		{
			m_matrix[i][j] += rhs.m_matrix[i][j];
			//chack if not bigger than 1000
			if (m_matrix[i][j] > 1000)
			{
				throw std::out_of_range("Matrix value is out of range");
			}
		}
	}
	return *this;
}

template <typename T>
SquareMatrix<T>& SquareMatrix<T>::operator-=(const SquareMatrix& rhs)
{
	for (int i = 0; i < m_size; ++i)
	{
		for (int j = 0; j < m_size; ++j)
		{
			m_matrix[i][j] -= rhs.m_matrix[i][j];
			//chack if not small than -1024
			if (m_matrix[i][j] < -1024)
			{
				throw std::out_of_range("Matrix value is out of range");
			}
		}
	}
	return *this;
}

template <typename T>
SquareMatrix<T> SquareMatrix<T>::Transpose() const
{
	SquareMatrix result(m_size);
	for (int i = 0; i < m_size; ++i)
	{
		for (int j = 0; j < m_size; ++j)
		{
			result(i, j) = m_matrix[j][i];
		}
	}
	return result;
}
template <typename T>
SquareMatrix<T> SquareMatrix<T>::operator*(const T& scalar) const
{
	SquareMatrix result(*this);
	for (int i = 0; i < m_size; ++i)
	{
		for (int j = 0; j < m_size; ++j)
		{
			result(i, j) *= scalar;
			//chack if not small than -1024 or bigger than 1000
			if (result(i, j) < -1024 || result(i, j) > 1000)
			{
				throw std::out_of_range("Matrix value is out of range");
			}
		}
	}
	return result;
}