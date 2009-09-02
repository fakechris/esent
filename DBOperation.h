#pragma once

class DbTransaction;
class DbOperationEnvironment;

class DbOperation
{
private:
	DbOperationEnvironment* env_;
public:
	JET_ERR on_commit(DbTransaction*) {return 0;}
	void set_environment(DbOperationEnvironment* env)
	{
		env_ = env;
	}

	JET_ERR execute()
	{
		// TODO: chris
		return 0;
	}
};