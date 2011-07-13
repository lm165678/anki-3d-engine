#include "TimeQuery.h"
#include "Util/Assert.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
TimeQuery::TimeQuery()
{
	glGenQueries(2, &glIds[0]);
	state = S_CREATED;
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
TimeQuery::~TimeQuery()
{
	glDeleteQueries(2, &glIds[0]);
}


//==============================================================================
// begin                                                                       =
//==============================================================================
void TimeQuery::begin()
{
	ASSERT(state == S_CREATED || state == S_ENDED);

	glQueryCounter(GL_TIMESTAMP, glIds[0]);

	state = S_STARTED;
}


//==============================================================================
// end                                                                         =
//==============================================================================
double TimeQuery::end()
{
	ASSERT(state == S_STARTED);

	glQueryCounter(GL_TIMESTAMP, glIds[1]);

	// Wait
	GLint done = 0;
	while(!done)
	{
		glGetQueryObjectiv(glIds[1], GL_QUERY_RESULT_AVAILABLE, &done);
	}

	// Get elapsed time
	GLuint64 timerStart, timerEnd;

	glGetQueryObjectui64v(glIds[0], GL_QUERY_RESULT, &timerStart);
	glGetQueryObjectui64v(glIds[1], GL_QUERY_RESULT, &timerEnd);

	state = S_ENDED;

	return (timerEnd - timerStart) / 1000000000.0;
}