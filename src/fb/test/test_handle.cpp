#include "gtest/gtest.h"
#include "stdint.h"
#include "fb_cpp_def.h"
#include "fbt_axis_virtual.h"


using namespace zmc;

inline uint32_t pointer_to_handle(void* ptr){
	return ((uint64_t) ptr) & 0xffffffff;
}
inline void* handle_to_pointer(uint32_t handle, void* basePtr){
	return (void*)((uint64_t) handle |(((uint64_t) basePtr) & 0xffffffff00000000));
}

TEST(handle, demo){
	AxisVirtual axis;
	FbtMoveAbsolute fbMov;
	auto handle = pointer_to_handle(&axis);
	// fbMov.runCycle();

	void* ptr = handle_to_pointer(handle, &fbMov);
	EXPECT_EQ(ptr, &axis);
}
