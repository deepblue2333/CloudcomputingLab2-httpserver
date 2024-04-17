#include <CUnit/Basic.h>

int maxi(int i1, int i2) {
    return (i1 > i2) ? i1 : i2;
}

void test_maxi(void) {
    CU_ASSERT(maxi(0, 2) == 2);
    CU_ASSERT(maxi(0, -2) == 0);
    CU_ASSERT(maxi(2, 2) == 2);
}

int main() {
    // 初始化CUnit测试注册表
    CU_initialize_registry();

    // 创建测试套件
    CU_pSuite suite = CU_add_suite("Maxi_Test_Suite", NULL, NULL);
    if (NULL == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // 向套件中添加测试用例
    if (NULL == CU_add_test(suite, "test of maxi function", test_maxi)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // 设置CUnit测试运行模式并运行测试
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    // 清理CUnit测试注册表
    CU_cleanup_registry();

    return CU_get_error();
}
