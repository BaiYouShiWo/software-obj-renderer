#pragma once
#include <iostream>
#include <vector>
#include <iomanip>
#include <math.h>

#define PI 3.14159265358979323846

class vec3{
public:
    float x, y, z;

    vec3() : x(0), y(0), z(0) {}
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }

    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }

    vec3 operator*(float scalar) const {
        return vec3(x * scalar, y * scalar, z * scalar);
    }

    vec3 operator/(float scalar) const {
        return vec3(x / scalar, y / scalar, z / scalar);
    }

    float dot(const vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    vec3 cross(const vec3& other) const {
        return vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    vec3 normalize() const {
        float len = length();
        return vec3(x / len, y / len, z / len);
    }
};

template<typename T>
class vec2{
public:
    T x, y;

    vec2() : x(0), y(0) {}
    vec2(T x, T y) : x(x), y(y){}

    vec2 operator+(const vec2& other) const {
        return vec2(x + other.x, y + other.y);
    }

    vec2 operator-(const vec2& other) const {
        return vec2(x - other.x, y - other.y);
    }

    vec2 operator*(float scalar) const {
        return vec2(x * scalar, y * scalar);
    }

    vec2 operator/(float scalar) const {
        return vec2(x / scalar, y / scalar);
    }

    float dot(const vec2& other) const {
        return x * other.x + y * other.y;
    }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    vec2 normalize() const {
        float len = length();
        return vec2(x / len, y / len);
    }
};

using Point2D = vec2<float>;
using Pixel2D = vec2<int>;

class vec4{
public:
    float x, y, z, w;

    vec4() : x(0), y(0), z(0), w(0) {}
    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    vec4 operator+(const vec4& other) const {
        return vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    vec4 operator-(const vec4& other) const {
        return vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    vec4 operator*(float scalar) const {
        return vec4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    vec4 operator/(float scalar) const {
        return vec4(x / scalar, y / scalar, z / scalar, w / scalar);
    }

    float dot(const vec4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    vec4 normalize() const {
        float len = length();
        return vec4(x / len, y / len, z / len, w / len);
    }

    vec3 homogenizate() const {
        auto r = 1 / w;
        return vec3(x * r, y * r, z * r);
    }
};
inline vec4 transform4(float a,float b,float c,float d){
    return vec4(a,b,c,d);
}


class Matrix {
public:
    Matrix(int rows, int cols) : rows_(rows), cols_(cols), data_(cols * rows) {
        if (rows <= 0 || cols <= 0) throw std::invalid_argument("Matrix dimensions must be positive");
        std::fill(data_.begin(), data_.end(), 0);
    }


    Matrix(const Matrix&) = default;
    Matrix& operator=(const Matrix&) = default;

    int rows() const noexcept { return rows_; }
    int cols() const noexcept { return cols_; }
    const std::vector<float>& data() const noexcept { return data_; }

    float&       operator()(int r, int c)       noexcept { return data_[c * rows_ + r]; }
    float        operator()(int r, int c) const noexcept { return data_[c * rows_ + r]; }

    Matrix T() noexcept {
        Matrix R(cols_, rows_);
        for (int i = 0; i < rows_; ++i)
            for (int j = i + 1; j < cols_; ++j)
                R(j, i) = data_[i * cols_ + j];
        return R;
        }

    void dump(std::ostream& os = std::cout, int prec = 4) const {
        std::ios_base::sync_with_stdio(false);
        os.tie(nullptr);

        const int buf_size = 32;
        thread_local std::unique_ptr<char[]> buf{new char[buf_size]};

        os << "Matrix " << rows_ << "x" << cols_ << ":\n";
        const float* p = data_.data();
        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols_; ++j) {
                const float v = p[i * cols_ + j];
                auto [ptr, ec] = std::to_chars(buf.get(), buf.get() + buf_size - 1, v,
                                            std::chars_format::fixed, prec);
                *ptr = ' ';
                os.write(buf.get(), ptr - buf.get() + 1);
            }
            os << '\n';
        }
    }

    void fill(float num)  {
        for (int i = 0; i < rows_; ++i)
            for (int j = 0; j < cols_; ++j)
                data_[i * cols_ + j] = num;
    }

private:
    int rows_, cols_;
    std::vector<float> data_;

    void check(int r, int c) const {
        if (r < 0 || r >= rows_ || c < 0 || c >= cols_)
            throw std::out_of_range("Matrix index out of range");
    }
};

inline Matrix operator+(const Matrix& A, const Matrix& B) {
    if (A.rows() != B.rows() || A.cols() != B.cols())
        throw std::invalid_argument("Matrix addition dimension mismatch");
    Matrix R(A.rows(), A.cols());
    for (int i = 0; i < A.rows(); ++i)
        for (int j = 0; j < A.cols(); ++j)
            R(i, j) = A(i, j) + B(i, j);
    return R;
}

inline Matrix operator-(const Matrix& A, const Matrix& B) {
    if (A.rows() != B.rows() || A.cols() != B.cols())
        throw std::invalid_argument("Matrix subtraction dimension mismatch");
    Matrix R(A.rows(), A.cols());
    for (int i = 0; i < A.rows(); ++i)
        for (int j = 0; j < A.cols(); ++j)
            R(i, j) = A(i, j) - B(i, j);
    return R;
}

inline Matrix operator*(const Matrix& A, const Matrix& B) {
    if (A.cols() != B.rows())
        throw std::invalid_argument("Matrix multiplication dimension mismatch");
    Matrix R(A.rows(), B.cols());
    for (int i = 0; i < A.rows(); ++i) {
        for (int k = 0; k < A.cols(); ++k) {
            float a = A(i, k);
            for (int j = 0; j < B.cols(); ++j) {
                R(i, j) += a * B(k, j);
            }
        }
    }
    return R;
}

inline Matrix operator*(float s, const Matrix& A) {
    Matrix R(A.rows(), A.cols());
    for (int i = 0; i < A.rows(); ++i)
        for (int j = 0; j < A.cols(); ++j)
            R(i, j) = s * A(i, j);
    return R;
}

inline Matrix operator*(const Matrix& A, float s) {
     return s * A; 
    }

inline void transform_batch_aos(
    vec4* out, const float* M,
    const float* x, const float* y, const float* z,
    size_t n) noexcept
{
    const __m128 c0 = _mm_load_ps(M + 0);
    const __m128 c1 = _mm_load_ps(M + 4);
    const __m128 c2 = _mm_load_ps(M + 8);
    const __m128 c3 = _mm_load_ps(M + 12);

    __m128 W = _mm_set1_ps(1.0f);
    for (size_t i = 0; i + 3 < n; i += 4) {

        __m128 X = _mm_load_ps(x + i); // [x0, x1, x2, x3]
        __m128 Y = _mm_load_ps(y + i); // [y0, y1, y2, y3]
        __m128 Z = _mm_load_ps(z + i); // [z0, z1, z2, z3]


        __m128 T0 = \
            _mm_fmadd_ps(_mm_shuffle_ps(X, X, _MM_SHUFFLE(0,0,0,0)), c0,
            _mm_fmadd_ps(_mm_shuffle_ps(Y, Y, _MM_SHUFFLE(0,0,0,0)), c1,
            _mm_fmadd_ps(_mm_shuffle_ps(Z, Z, _MM_SHUFFLE(0,0,0,0)), c2,
                         _mm_mul_ps(W, c3))));

        __m128 T1 = \
            _mm_fmadd_ps(_mm_shuffle_ps(X, X, _MM_SHUFFLE(1,1,1,1)), c0,
            _mm_fmadd_ps(_mm_shuffle_ps(Y, Y, _MM_SHUFFLE(1,1,1,1)), c1,
            _mm_fmadd_ps(_mm_shuffle_ps(Z, Z, _MM_SHUFFLE(1,1,1,1)), c2,
                         _mm_mul_ps(W, c3))));

        __m128 T2 = \
            _mm_fmadd_ps(_mm_shuffle_ps(X, X, _MM_SHUFFLE(2,2,2,2)), c0,
            _mm_fmadd_ps(_mm_shuffle_ps(Y, Y, _MM_SHUFFLE(2,2,2,2)), c1,
            _mm_fmadd_ps(_mm_shuffle_ps(Z, Z, _MM_SHUFFLE(2,2,2,2)), c2,
                         _mm_mul_ps(W, c3))));

        __m128 T3 = \
            _mm_fmadd_ps(_mm_shuffle_ps(X, X, _MM_SHUFFLE(3,3,3,3)), c0,
            _mm_fmadd_ps(_mm_shuffle_ps(Y, Y, _MM_SHUFFLE(3,3,3,3)), c1,
            _mm_fmadd_ps(_mm_shuffle_ps(Z, Z, _MM_SHUFFLE(3,3,3,3)), c2,
                         _mm_mul_ps(W, c3))));

        _mm_storeu_ps((float*)out + 4*i, T0);
        _mm_storeu_ps((float*)out + 4*i + 4, T1);
        _mm_storeu_ps((float*)out + 4*i + 8, T2);
        _mm_storeu_ps((float*)out + 4*i + 12, T3);
    }

    for (size_t i = n & ~3; i < n; ++i) {
        float x_ = M[0]*x[i] + M[4]*y[i] + M[ 8]*z[i] + M[12];
        float y_ = M[1]*x[i] + M[5]*y[i] + M[ 9]*z[i] + M[13];
        float z_ = M[2]*x[i] + M[6]*y[i] + M[10]*z[i] + M[14];
        float w_ = M[3]*x[i] + M[7]*y[i] + M[11]*z[i] + M[15];
        out[i] = vec4{ x_, y_, z_, w_ };
    }
}


Matrix getRotateMatrix(float x_angle, float y_angle, float z_angle){
    Matrix Rx(4,4);
    Rx(0,0) = 1; Rx(0,1) = 0;                Rx(0,2) = 0;                 Rx(0,3) = 0;
    Rx(1,0) = 0; Rx(1,1) = cos(x_angle);     Rx(1,2) = -sin(x_angle);     Rx(1,3) = 0;
    Rx(2,0) = 0; Rx(2,1) = sin(x_angle);     Rx(2,2) = cos(x_angle);      Rx(2,3) = 0;
    Rx(3,0) = 0; Rx(3,1) = 0;                Rx(3,2) = 0;                 Rx(3,3) = 1;

    Matrix Ry(4,4);
    Ry(0,0) = cos(y_angle);   Ry(0,1) = 0; Ry(0,2) = sin(y_angle);    Ry(0,3) = 0;
    Ry(1,0) = 0;              Ry(1,1) = 1; Ry(1,2) = 0;               Ry(1,3) = 0;
    Ry(2,0) = -sin(y_angle);  Ry(2,1) = 0; Ry(2,2) = cos(y_angle);    Ry(2,3) = 0;
    Ry(3,0) = 0;              Ry(3,1) = 0; Ry(3,2) = 0;               Ry(3,3) = 1;

    Matrix Rz(4,4);
    Rz(0,0) = cos(z_angle);   Rz(0,1) = -sin(z_angle);    Rz(0,2) = 0; Rz(0,3) = 0;
    Rz(1,0) = sin(z_angle);   Rz(1,1) = cos(z_angle);     Rz(1,2) = 0; Rz(1,3) = 0;
    Rz(2,0) = 0;              Rz(2,1) = 0;                Rz(2,2) = 1; Rz(2,3) = 0;
    Rz(3,0) = 0;              Rz(3,1) = 0;                Rz(3,2) = 0; Rz(3,3) = 1;

    return Rz * Ry * Rx;
}

Matrix getTranslateMatrix(float tx, float ty, float tz){
    Matrix T(4,4);
    T(0,0) = 1; T(0,1) = 0; T(0,2) = 0; T(0,3) = tx;
    T(1,0) = 0; T(1,1) = 1; T(1,2) = 0; T(1,3) = ty;
    T(2,0) = 0; T(2,1) = 0; T(2,2) = 1; T(2,3) = tz;
    T(3,0) = 0; T(3,1) = 0; T(3,2) = 0; T(3,3) = 1;
    return T;
}

Matrix getViewMatrix(vec3 eye, vec3 center, vec3 up){
    vec3 f = (center - eye).normalize();
    vec3 s = f.cross(up).normalize();
    vec3 u = s.cross(f);

    Matrix V(4,4);
    V(0,0) = s.x; V(0,1) = s.y; V(0,2) = s.z; V(0,3) = -s.dot(eye);
    V(1,0) = u.x; V(1,1) = u.y; V(1,2) = u.z; V(1,3) = -u.dot(eye);
    V(2,0) = -f.x;V(2,1) = -f.y;V(2,2) = -f.z;V(2,3) = f.dot(eye);
    V(3,0) = 0;   V(3,1) = 0;   V(3,2) = 0;   V(3,3) = 1;
    return V;
}

Matrix getPerspectiveMatrix(float fov, float aspect, float _near, float _far){
    Matrix P(4,4);
    float f = 1.0f / tan(fov / 2.0f);
    P(0,0) = f / aspect; P(0,1) = 0;   P(0,2) = 0;                          P(0,3) = 0;
    P(1,0) = 0;          P(1,1) = f;   P(1,2) = 0;                          P(1,3) = 0;
    P(2,0) = 0;          P(2,1) = 0;   P(2,2) = (_far + _near) / (_near - _far); P(2,3) = (2 * _far * _near) / (_near - _far);
    P(3,0) = 0;          P(3,1) = 0;   P(3,2) = -1;                         P(3,3) = 0;
    return P;
}


