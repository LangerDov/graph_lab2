class Config
{
	friend class Core;
public:
	
private:
	bool Init();
	bool CleanUp();
};

extern Config* config;