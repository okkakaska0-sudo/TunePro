#pragma once
// Eigen library stub for matrix operations
#include <vector>
#include <array>
#include <cmath>

namespace Eigen {
    template<int Rows, int Cols>
    class Matrix {
    public:
        std::array<float, Rows * Cols> data;
        
        Matrix() { data.fill(0.0f); }
        
        float& operator()(int row, int col) {
            return data[row * Cols + col];
        }
        
        const float& operator()(int row, int col) const {
            return data[row * Cols + col];
        }
        
        Matrix operator*(const Matrix& other) const {
            Matrix result;
            for (int i = 0; i < Rows; ++i) {
                for (int j = 0; j < Cols; ++j) {
                    float sum = 0.0f;
                    for (int k = 0; k < Cols; ++k) {
                        sum += (*this)(i, k) * other(k, j);
                    }
                    result(i, j) = sum;
                }
            }
            return result;
        }
    };
    
    using Matrix3f = Matrix<3, 3>;
    using Matrix4f = Matrix<4, 4>;
    using VectorXf = std::vector<float>;
}