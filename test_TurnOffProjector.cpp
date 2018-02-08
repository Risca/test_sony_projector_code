#include "gmock/gmock.h"

#include "serial.h"
#include "projector.h"

#include <vector>

using namespace testing;

#define START_CODE ((char)0xA9)
#define END_CODE   ((char)0x9A)

MATCHER_P(IsCmd, cmd, "") {
	return std::equal(cmd.begin(), cmd.end(), arg);
}

class TurnOffProjectorTest : public Test
{
public:
	TurnOffProjectorTest() :
		m_StatusCmd{START_CODE, 0x01, 0x02, 0x01, 0x00, 0x00, 0x03, END_CODE},
		m_PowerOffCmd{START_CODE, 0x17, 0x2F, 0x00, 0x00, 0x00, 0x3F, END_CODE}
	{
	}

	void SetUp() {
		SerialProxy::g_Instance = new SerialMock;
		EXPECT_CALL(*SerialProxy::g_Instance, write(IsCmd(m_StatusCmd), 8))
			.WillRepeatedly(Return(8));
		EXPECT_CALL(*SerialProxy::g_Instance, print(NotNull()))
			.WillRepeatedly(Return(0));
		EXPECT_CALL(*SerialProxy::g_Instance, println(NotNull()))
			.WillRepeatedly(Return(0));
	}

	void TearDown() {
		delete SerialProxy::g_Instance;
		SerialProxy::g_Instance = 0;
	}

protected:
	const std::array<char, 8> m_StatusCmd, m_PowerOffCmd;
};

TEST_F(TurnOffProjectorTest, AlreadyOff)
{
	const std::array<char, 8> response =
		{ START_CODE, 0x01, 0x02, 0x02, 0x00, STATE_STANDBY, 0x03 | STATE_STANDBY, END_CODE };
	EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
		.WillOnce(DoAll(SetArrayArgument<0>(response.begin(), response.end()),
				Return(8)));
	TurnOffProjector();
}

TEST_F(TurnOffProjectorTest, TurnOffDirectly)
{
	std::array<char, 8> responses[] = {
		{ START_CODE, 0x01, 0x02, 0x02, 0x00, STATE_POWER_ON, 0x03 | STATE_POWER_ON, END_CODE },
		{ START_CODE, 0x01, 0x02, 0x02, 0x00, STATE_STANDBY, 0x03 | STATE_STANDBY, END_CODE },
	};

	Expectation initialState =
		EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
			.WillOnce(DoAll(SetArrayArgument<0>(responses[0].begin(), responses[0].end()),
					Return(8)));
	Expectation turnOffCommand =
		EXPECT_CALL(*SerialProxy::g_Instance, write(IsCmd(m_PowerOffCmd), 8))
			.After(initialState)
			.WillOnce(Return(8));

	EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
		.After(turnOffCommand)
		.WillOnce(DoAll(SetArrayArgument<0>(responses[1].begin(), responses[1].end()),
				Return(8)));
	TurnOffProjector();
}

TEST_F(TurnOffProjectorTest, TurnOffSlowly)
{
	std::array<char, 8> responses[] = {
		{ START_CODE, 0x01, 0x02, 0x02, 0x00, STATE_POWER_ON, 0x03 | STATE_POWER_ON, END_CODE },
		{ START_CODE, 0x01, 0x02, 0x02, 0x00, STATE_COOLING_1, 0x03 | STATE_COOLING_1, END_CODE },
		{ START_CODE, 0x01, 0x02, 0x02, 0x00, STATE_COOLING_2, 0x03 | STATE_COOLING_2, END_CODE },
		{ START_CODE, 0x01, 0x02, 0x02, 0x00, STATE_STANDBY, 0x03 | STATE_STANDBY, END_CODE },
	};

	Expectation initialState =
		EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
			.WillOnce(DoAll(SetArrayArgument<0>(responses[0].begin(), responses[0].end()),
					Return(8)));
	Expectation turnOffCommand =
		EXPECT_CALL(*SerialProxy::g_Instance, write(IsCmd(m_PowerOffCmd), 8))
			.After(initialState)
			.WillOnce(Return(8));

	Expectation firstTransition =
		EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
			.Times(2)
			.After(turnOffCommand)
			.WillRepeatedly(DoAll(SetArrayArgument<0>(responses[1].begin(), responses[1].end()),
					      Return(8)));

	Expectation secondTransition =
		EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
			.Times(2)
			.After(firstTransition)
			.WillRepeatedly(DoAll(SetArrayArgument<0>(responses[2].begin(), responses[2].end()),
					      Return(8)));

	EXPECT_CALL(*SerialProxy::g_Instance, readBytes(NotNull(), 8))
		.After(secondTransition)
		.WillOnce(DoAll(SetArrayArgument<0>(responses[3].begin(), responses[3].end()),
				Return(8)));

	TurnOffProjector();
}

int main( int argc, char * argv[] )
{
	srand (time(NULL));
	InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

