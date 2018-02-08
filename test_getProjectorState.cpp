#include "gmock/gmock.h"

#include "serial.h"
#include "projector.h"

#include <array>

using namespace testing;

#define START_CODE ((char)0xA9)
#define END_CODE   ((char)0x9A)

MATCHER_P(IsCmd, cmd, "") {
	return std::equal(cmd.begin(), cmd.end(), arg);
}

class ProjectorStateTest : public TestWithParam<int>
{
public:
	ProjectorStateTest() :
		m_StatusCmd{START_CODE, 0x01, 0x02, 0x01, 0x00, 0x00, 0x03, END_CODE}
	{
	}

	void SetUp() {
		SerialProxy::g_Instance = new SerialMock;
		EXPECT_CALL(*SerialProxy::g_Instance, write(IsCmd(m_StatusCmd), 8))
			.WillOnce(Return(8));
	}

	void TearDown() {
		delete SerialProxy::g_Instance;
		SerialProxy::g_Instance = 0;
	}

protected:
	const std::array<char, 8> m_StatusCmd;
};

TEST_F(ProjectorStateTest, SerialTimeout)
{
	EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
		.WillOnce(Return(0));
	EXPECT_EQ(GetProjectorState(), STATE_UNKNOWN);
}

TEST_P(ProjectorStateTest, WrongByte)
{
	int b = GetParam();
	std::array<char, 9> response = { START_CODE, 0x01, 0x02, 0x02, 0x00, 0x00, 0x03, END_CODE, 0x00 };
	response[b] = 0xFF;
	EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
		.WillOnce(DoAll(SetArrayArgument<0>(response.begin(), response.begin() + 8),
				Return(8)));
	if (b < 8) {
		EXPECT_EQ(GetProjectorState(), STATE_UNKNOWN);
	}
	else {
		EXPECT_EQ(GetProjectorState(), static_cast<ProjectorState>(0));
	}
}

TEST_P(ProjectorStateTest, Ok)
{
	ProjectorState state = static_cast<ProjectorState>(GetParam());
	const std::array<char, 8> response = { START_CODE, 0x01, 0x02, 0x02, 0x00, state, char(0x03 | state), END_CODE };
	EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
		.WillOnce(DoAll(SetArrayArgument<0>(response.begin(), response.end()),
				Return(8)));
	EXPECT_EQ(GetProjectorState(), state);
}

INSTANTIATE_TEST_CASE_P(GetState, ProjectorStateTest, Range(0, 9));

int main( int argc, char * argv[] )
{
	srand (time(NULL));
	InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

