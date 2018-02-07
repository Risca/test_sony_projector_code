#include "gmock/gmock.h"

#include "serial.h"
#include "projector.h"

#include <vector>

using namespace testing;

MATCHER_P(IsCmd, cmd, "") {
	return std::equal(cmd.begin(), cmd.end(), arg);
}

class ProjectorStateTest : public TestWithParam<int>
{
public:
	void SetUp() {
		m_StatusCmd.push_back(0xA9); // start
		m_StatusCmd.push_back(0x01); // status
		m_StatusCmd.push_back(0x02); // power
		m_StatusCmd.push_back(0x01); // GET
		m_StatusCmd.push_back(0x00); // dummy
		m_StatusCmd.push_back(0x00); // dummy
		m_StatusCmd.push_back(0x03); // checksum
		m_StatusCmd.push_back(0x9A); // end

		SerialProxy::g_Instance = new SerialMock;
		EXPECT_CALL(*SerialProxy::g_Instance, write(IsCmd(m_StatusCmd), 8))
			.WillOnce(Return(8));
	}
	void TearDown() {
		delete SerialProxy::g_Instance;
		SerialProxy::g_Instance = 0;
	}

protected:
	std::vector<char> m_StatusCmd;
};

TEST_F(ProjectorStateTest, SerialTimeout)
{
	EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
		.WillOnce(Return(0));
	EXPECT_EQ(GetProjectorState(), 0);
}

TEST_P(ProjectorStateTest, WrongByte)
{
	byte b = GetParam();
	char response[9] = { 0xA9, 0x01, 0x02, 0x02, 0x00, 0x00, 0x03, 0x9A, 0x00 };
	response[b] = 0xFF;
	EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
		.WillOnce(DoAll(SetArrayArgument<0>(&response[0], &response[8]),
				Return(8)));
	EXPECT_EQ(GetProjectorState(), 0);
}

TEST_P(ProjectorStateTest, Ok)
{
	byte state = GetParam();
	char response[8] = { 0xA9, 0x01, 0x02, 0x02, 0x00, state, 0x03 | state, 0x9A };
	EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
		.WillOnce(DoAll(SetArrayArgument<0>(&response[0], &response[8]),
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

