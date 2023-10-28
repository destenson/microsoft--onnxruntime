/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Licensed under the MIT License.

Module Name:

    test_sqnbitgemm.h

Abstract:

    Tests for MLAS n-bit int block quantized GEMM.

--*/

#include "test_util.h"
#include "mlas_qnbit.h"

/**
 * @brief Test class for n-bit int block quantized GEMM
 *        Note: only 2-D matmul supported for now
 */
template <size_t BlkBitWidth, size_t BlkLen>
class MlasSQNBitGemmTest : public MlasTestBase {
 private:
  MatrixGuardBuffer<float> BufferA;
  MatrixGuardBuffer<float> BufferB;
  MatrixGuardBuffer<uint8_t> BufferPackedBData;
  MatrixGuardBuffer<uint8_t> BufferPackedBZeroPoint;
  MatrixGuardBuffer<float> BufferPackedBScale;
  MatrixGuardBuffer<float> BufferUnpackedBReference;
  MatrixGuardBuffer<float> BufferBias;
  MatrixGuardBuffer<float> BufferC;
  MatrixGuardBuffer<float> BufferCReference;

  void CallGemm(size_t M,
                size_t N,
                size_t K,
                const float* A,
                size_t lda,
                const uint8_t* PackedBData,
                const float* PackedBScale,
                const uint8_t* PackedBZeroPoint,
                const float* Bias,
                float* C,
                size_t ldc,
                MLAS_THREADPOOL* Threadpool) {
    MLAS_SQNBIT_GEMM_DATA_PARAMS params;
    params.A = A;
    params.lda = lda;
    params.Bias = Bias;
    params.C = C;
    params.ldc = ldc;
    params.PackedBData = PackedBData;
    params.PackedBScale = PackedBScale;
    params.PackedBZeroPoint = PackedBZeroPoint;
    params.PostProcessor = nullptr;

    MlasSQNBitGemmBatch(M, N, K, 1, BlkBitWidth, BlkLen, &params, Threadpool);
  }

  void CallReferenceGemm(size_t M,
                         size_t N,
                         size_t K,
                         const float* A,
                         const uint8_t* PackedBData,
                         const float* PackedBScale,
                         const uint8_t* PackedBZeroPoint,
                         const float* Bias,
                         float* C) {
    float* UnpackedBData = BufferUnpackedBReference.GetBuffer(K * N);
    MlasReferenceQNBitPacking<BlkBitWidth, BlkLen>::UnpackB(
        N, K, PackedBData, PackedBScale, PackedBZeroPoint, UnpackedBData, N);

    for (size_t m = 0; m < M; m++) {
      for (size_t n = 0; n < N; n++) {
        const float* a = A + m * K;
        const float* b = UnpackedBData + n;
        float* c = C + (m * N) + n;

        float sum = Bias == nullptr ? 0.0f : Bias[n];
        for (size_t k = 0; k < K; k++) {
          sum += (*a) * (*b);
          b += N;
          a += 1;
        }
        *c = sum;
      }
    }
  }

 public:
  void Test(size_t M, size_t N, size_t K,
            bool WithBias, bool Symmetric, bool WithThreadpool) {
    MLAS_THREADPOOL* Threadpool = WithThreadpool ? GetMlasThreadPool() : nullptr;

    const float* A = BufferA.GetBuffer(K * M);

    const float* B = BufferB.GetBuffer(N * K);

    const float* Bias = nullptr;
    if (WithBias) {
      Bias = BufferBias.GetBuffer(N);
    }

#if 0
    auto print_matrix = [](size_t ncols, size_t nrows, const float* data) {
      for (size_t row = 0; row < nrows; ++row) {
        for (size_t col = 0; col < ncols; ++col) {
          std::cout << data[row * nrows + col] << "\t";
        }
        std::cout << "\n";
      }
    };

    std::cout << "A:\n";
    print_matrix(M, K, A);
    std::cout << "B:\n";
    print_matrix(K, N, B);
#endif

    float* C = BufferC.GetBuffer(N * M, true);
    float* CReference = BufferCReference.GetBuffer(N * M, true);

    // pack B
    uint8_t* PackedBData = nullptr;
    float* PackedBScale = nullptr;
    uint8_t* PackedBZeroPoint = nullptr;
    {
      size_t PackedBDataSize, PackedBScaleSize, PackedBZeroPointSize;
      MlasReferenceQNBitPacking<BlkBitWidth, BlkLen>::GetPackedBSizes(
          N, K, PackedBDataSize, PackedBScaleSize, &PackedBZeroPointSize);

      PackedBData = BufferPackedBData.GetBuffer(PackedBDataSize);
      PackedBScale = BufferPackedBScale.GetBuffer(PackedBScaleSize);
      if (Symmetric) {
        PackedBZeroPoint = BufferPackedBZeroPoint.GetBuffer(PackedBZeroPointSize);
      }

      MlasReferenceQNBitPacking<BlkBitWidth, BlkLen>::PackB(N, K, B, /* ldb */ N,
                                                            PackedBData, PackedBScale, PackedBZeroPoint);
    }

    CallGemm(M, N, K, A, /* lda */ K, PackedBData, PackedBScale, PackedBZeroPoint, Bias, C, /* ldc */ N, Threadpool);
    CallReferenceGemm(M, N, K, A, PackedBData, PackedBScale, PackedBZeroPoint, Bias, CReference);

    size_t f = 0;
    for (size_t m = 0; m < M; m++) {
      for (size_t n = 0; n < N; n++, f++) {
        ASSERT_TRUE(CloseEnough(C[f], CReference[f]))
            << "Expected: " << CReference[f] << " Actual: " << C[f] << "@[" << m << "x" << n << "], "
            << "M=" << M << ", N=" << N << ", K=" << K;
      }
    }
  }

 public:
  static const char* GetTestSuiteName() {
    static std::string suite_name = std::string("SQNBitGemm") +
                                    "BlkBitWidth" + std::to_string(BlkBitWidth) +
                                    "BlkLen" + std::to_string(BlkLen);
    return suite_name.c_str();
  }
};

//
// Short Execute() test helper to register each test separately by all parameters.
//
template <size_t BlkBitWidth, size_t BlkLen>
class SQNBitGemmShortExecuteTest : public MlasTestFixture<MlasSQNBitGemmTest<BlkBitWidth, BlkLen>> {
 public:
  explicit SQNBitGemmShortExecuteTest(size_t M, size_t N, size_t K,
                                      bool WithThreadpool, bool Symmetric, bool WithBias)
      : M_(M), N_(N), K_(K), WithThreadpool_(WithThreadpool), Symmetric_(Symmetric), WithBias_(WithBias) {
  }

  void TestBody() override {
    MlasTestFixture<MlasTesterType>::mlas_tester->Test(
        M_, N_, K_, WithThreadpool_, Symmetric_, WithBias_);
  }

  static size_t RegisterSingleTest(size_t M, size_t N, size_t K,
                                   bool WithThreadpool, bool Symmetric, bool WithBias) {
    std::stringstream ss;
    ss << (WithThreadpool ? "SingleThread" : "Threaded")
       << "/isSymmetric" << Symmetric
       << "/M" << M << "xN" << N << "xK" << K
       << "/hasBias" << WithBias;
    auto test_name = ss.str();

    testing::RegisterTest(
        MlasTesterType::GetTestSuiteName(),
        test_name.c_str(),
        nullptr,
        test_name.c_str(),
        __FILE__,
        __LINE__,
        // Important to use the fixture type as the return type here.
        [=]() -> MlasTestFixture<MlasTesterType>* {
          return new SQNBitGemmShortExecuteTest(
              M, N, K, WithThreadpool, Symmetric, WithBias);
        });

    return 1;
  }

  static size_t RegisterShortExecuteTests() {
    size_t test_registered = 0;

    if (MlasIsSQNBitGemmAvailable(BlkBitWidth, BlkLen)) {
      for (bool WithThreadpool : {false, true}) {
        for (bool Symmetric : {false, true}) {
          for (size_t b = 1; b < 16; b++) {
            test_registered += RegisterSingleTest(b, b, b, WithThreadpool, Symmetric, false);
            test_registered += RegisterSingleTest(b, b, b, WithThreadpool, Symmetric, true);
          }
          for (size_t b = 16; b <= 256; b <<= 1) {
            test_registered += RegisterSingleTest(b, b, b, WithThreadpool, Symmetric, false);
            test_registered += RegisterSingleTest(b, b, b, WithThreadpool, Symmetric, true);
          }
          for (size_t b = 256; b < 320; b += 32) {
            test_registered += RegisterSingleTest(b, b, b, WithThreadpool, Symmetric, true);
          }
          for (size_t b = 1; b < 96; b++) {
            test_registered += RegisterSingleTest(1, b, 32, WithThreadpool, Symmetric, false);
            test_registered += RegisterSingleTest(1, 32, b, WithThreadpool, Symmetric, true);
            test_registered += RegisterSingleTest(1, b, b, WithThreadpool, Symmetric, false);
          }
          test_registered += RegisterSingleTest(43, 500, 401, WithThreadpool, Symmetric, true);

          // test_registered += RegisterSingleTest(1001, 1027, 1031, WithThreadpool, Symmetric, false);
        }
      }
    }

    return test_registered;
  }

 private:
  size_t M_, N_, K_;
  bool WithThreadpool_, Symmetric_, WithBias_;
};

#define DEFINE_MLAS_TESTER(BlkBitWidth, BlkLen) \
  template <>                                   \
  MlasSQNBitGemmTest<BlkBitWidth, BlkLen>* MlasTestFixture<MlasSQNBitGemmTest<BlkBitWidth, BlkLen>>::mlas_tester(nullptr)

DEFINE_MLAS_TESTER(4, 16);
DEFINE_MLAS_TESTER(4, 32);
DEFINE_MLAS_TESTER(4, 64);

#undef DEFINE_MLAS_TESTER

static size_t SQNBitGemmRegisterAllShortExecuteTests() {
  size_t count = 0;

  count += SQNBitGemmShortExecuteTest<4, 16>::RegisterShortExecuteTests();
  count += SQNBitGemmShortExecuteTest<4, 32>::RegisterShortExecuteTests();
  count += SQNBitGemmShortExecuteTest<4, 64>::RegisterShortExecuteTests();

  return count;
}

static UNUSED_VARIABLE bool added_to_main = AddTestRegister([](bool is_short_execute) {
  if (is_short_execute) {
    return SQNBitGemmRegisterAllShortExecuteTests() > 0;
  }
  return false;
});
