#include <iostream>
#include <string>
#include <algorithm>
#include <ctime>
#include <vector>
#include <chrono>
#include <random>
#include <regex>
#include <sstream>

#define DATE_BUFFER 80

using namespace std;

enum ERRORS
{
	NO_PLAYLIST_FOUND = -1,
	INVALID_TRACK = -3,
	INVALID_CREATION_ELEMENTS = -4,
	UNKNOWN_ERROR = -5
};

class PlaylistContents
{
public:
	vector<int> m_ordinal;
	int m_curPlayingIdx;

	PlaylistContents(int numTracks)
	{
		Create(numTracks); 
	}

	int Create(int numTracks)
	{
		if (m_ordinal.size() > 0)
		{
			m_ordinal.clear(); 
		}

		for (int i = 1; i <= numTracks; i++)
		{
			m_ordinal.push_back(i);
		}
		m_curPlayingIdx = 0;
		return 0; 
	}

	int resizeList(int newCount)
	{
		m_ordinal.resize(newCount);
	}

	int insert(int ordinal, int trackID)
	{
		vector<int>::iterator it;
		if (0 <= ordinal || ordinal > (m_ordinal).size())
		{
			return INVALID_TRACK;
		}
		else
		{
			it = (m_ordinal).begin() + ordinal - 1;
			(m_ordinal).insert(it, trackID);
		}
	}

	int Play(int trackID)
	{
		if (trackID > m_ordinal.size() || trackID <= 0)
		{
			return INVALID_TRACK;
		}
		else
		{
			m_curPlayingIdx = trackID; 
		}
		return 0; 
	}

	int Delete(int trackID)
	{
		if (trackID > m_ordinal.size() || trackID <= 0)
		{
			return INVALID_TRACK;
		}
		else
		{
			// Take care of the currently playing index. 
			if (m_curPlayingIdx == trackID)
			{
				m_curPlayingIdx = 0; 
			}
			else
			{
				// Adjust currently playing index only if it's after the ordinal that's about to be removed. 
				if (m_curPlayingIdx > 0 && m_curPlayingIdx > trackID)
				{
					m_curPlayingIdx--; 
				}
			}

			// Remove the actual element. 
			m_ordinal.erase(m_ordinal.begin() + trackID - 1);
		}

		return 0; 
	}

	int Insert(int ordinal, int trackID)
	{
		if (ordinal > m_ordinal.size() || ordinal <= 0)
		{
			return INVALID_TRACK;
		}
		else
		{
			m_ordinal.insert(m_ordinal.begin() + (ordinal - 1), trackID); 

			// Only adjust if you are adding before the currently playing index. 
			if (m_curPlayingIdx >= ordinal)
			{
				m_curPlayingIdx++; 
			}
		}
		return 0; 
	}

	void PrintPlaylistContents()
	{
		{
			// One based. 
			int idx = 1;
			for (vector<int>::iterator it = (m_ordinal).begin(); it != (m_ordinal).end(); it++)
			{
				char c = (m_curPlayingIdx > 0 && m_curPlayingIdx == idx) ? '*' : ' '; 
				cout << c <<  *it << "  " ; 
				idx++; 
			}
			cout << endl; 
		}
	}

	int Shuffle()
	{
		int hr = 0; 
		try
		{
			int curElementPlaying = 0;

			// Save the element that's currently playing. 
			if (m_curPlayingIdx > 0)
			{
				vector<int>::iterator it = m_ordinal.begin() + (m_curPlayingIdx - 1);
				curElementPlaying = *it;
			}

			// Shuffle the list. 
			unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
			shuffle(m_ordinal.begin(), m_ordinal.end(), default_random_engine(seed));

			// Copy the element playing back to it's spot. 
			if (m_curPlayingIdx > 0)
			{
				vector<int>::iterator swappedElement = find(m_ordinal.begin(), m_ordinal.end(), curElementPlaying);
				iter_swap(m_ordinal.begin() + (m_curPlayingIdx - 1), swappedElement);
			}
		}
		catch (exception& e)
		{
			hr = UNKNOWN_ERROR;
		}
		
		return hr; 
	}

	int GetListSize()
	{
		return m_ordinal.size(); 
	}

};

class Playlist
{
public:
	string m_name;
	int m_trackCount;
	char m_lastModifiedTime[DATE_BUFFER];
	PlaylistContents * m_playlistContents; 


	Playlist(string playlist_name)
	{
		m_name = playlist_name;
		m_trackCount = 0;

		//Update last touched time
		UpdateModifiedTime(); 

		m_playlistContents = NULL;
	}

	int Create(int numTracks)
	{
		if (numTracks > 0)
		{
			AddPlaylistContents(numTracks);
			int hr = m_playlistContents->Create(numTracks);
			if (!hr)
			{
				UpdateModifiedTime();
				UpdateTrackCount();
			}
			return hr; 
		}
		return INVALID_CREATION_ELEMENTS;
	}

	int Delete(int ordinal)
	{
		if (!m_playlistContents)
		{
			return NO_PLAYLIST_FOUND;
		}
		if (ordinal > m_playlistContents->GetListSize())
		{
			return INVALID_TRACK;
		}
		int hr = m_playlistContents->Delete(ordinal);
		if (!hr)
		{
			UpdateModifiedTime();
			UpdateTrackCount();
		}
		return hr;
	}

	int Insert(int ordinal, int trackID)
	{
		if (!m_playlistContents)
		{
			return NO_PLAYLIST_FOUND;
		}
		if (ordinal > m_playlistContents->GetListSize())
		{
			return INVALID_TRACK;
		}
		int hr = m_playlistContents->Insert(ordinal, trackID);
		if (!hr)
		{
			UpdateModifiedTime();
			UpdateTrackCount();
		}
		return hr;
	}

	int Shuffle()
	{
		if (!m_playlistContents)
		{
			return NO_PLAYLIST_FOUND;
		}
		m_playlistContents->Shuffle();
		UpdateModifiedTime();
		return 0;
	}

	int Play(int trackID)
	{
		if (!m_playlistContents)
		{
			return NO_PLAYLIST_FOUND;
		}
		if (trackID > m_playlistContents->GetListSize())
		{
			return INVALID_TRACK;
		}
		m_playlistContents->Play(trackID); 
		return 0; 
	}

	int UpdateModifiedTime()
	{
		//Get Local Time & Format it to a char buffer.
		time_t raw_time;
		struct tm timeinfo;
		time(&raw_time);
		localtime_s(&timeinfo, &raw_time);

		strftime(m_lastModifiedTime, DATE_BUFFER, "%c", &timeinfo);
		return 0;
	}

	/* If not already allocated, then allocate else return error */
	int AddPlaylistContents(int num)
	{
		if (!m_playlistContents)
		{
			m_playlistContents = new PlaylistContents(num);
			return 0;
		}
		return INVALID_TRACK;
	}

	void UpdateTrackCount()
	{
		m_trackCount = m_playlistContents->GetListSize(); 
	}

	void PrintPlaylistInfo()
	{
		if (m_playlistContents)
		{
			cout << "Playlist Name : " << m_name << endl;
			cout << "Number of Entries: " << m_trackCount << endl;
			cout << "Last Modified: " << m_lastModifiedTime << endl;
			cout << "Playlist contents: " << endl;
			(*m_playlistContents).PrintPlaylistContents(); 
		}
		else
		{
			cout << "No Playlists Found. " << endl; 
		}
		cout.width(11);
		cout.fill('.');
		cout << "." << endl;
	}
};

class TestDriver
{
	public: 
	
		void TestCreate()
		{
			Playlist myPlayList("Workout");
			myPlayList.Create(0); 
			myPlayList.PrintPlaylistInfo();
			myPlayList.Create(5);
			myPlayList.Play(2);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Create(5);
			myPlayList.PrintPlaylistInfo();
		}

		void TestDelete()
		{
			Playlist myPlayList("Workout");
			myPlayList.Create(5);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(2);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Create(5);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Play(3); 
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(1);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(2);
			myPlayList.PrintPlaylistInfo();

		}

		void TestInsert()
		{
			Playlist myPlayList("Workout");
			myPlayList.Create(5);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Play(3);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Insert(3,4);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Insert(3, 9);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Insert(6, 8);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(2);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(4);
			myPlayList.PrintPlaylistInfo();
		}

		void TestShuffle()
		{
			Playlist myPlayList("Workout");
			myPlayList.Create(5);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Play(2);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Shuffle();
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(3);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Shuffle();
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(1);
			myPlayList.PrintPlaylistInfo();

		}

		void TestPlay()
		{
			Playlist myPlayList("Workout");
			myPlayList.AddPlaylistContents(5);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Play(4);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(2);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(1);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(2);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(4);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(1);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Delete(1);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Insert(1, 9);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Create(5);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Play(5);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Shuffle();
			myPlayList.PrintPlaylistInfo();
			myPlayList.Play(2);
			myPlayList.PrintPlaylistInfo();
			myPlayList.Shuffle();
			myPlayList.PrintPlaylistInfo();
		}
};

class TestStdIn
{
	public: 
		TestStdIn()
		{
			Playlist myPlayList("Workout");
			cout << "Valid ops are create <N>, play <N>, insert <N><X>, delete <N>, shuffle" << endl; 
			while (1)
			{
				cout << ">>>> " ; 
				vector<string> tokens; 
				string item; 
				string item2; 
				getline(cin, item); 
				stringstream ss(item); 
				while (getline(ss, item2, ' '))
				{
					tokens.push_back(item2); 
				}

				if (tokens.size() > 0)
				{
					string operation = tokens[0]; 
					for (int i = 0; i < operation.size(); i++)
					{
						operation.at(i) = tolower(operation.at(i)); 
					}
					if ( (operation.compare("create") == 0) && (tokens.size() > 1) )
					{
						int numElements = stoi(tokens[1]); 
						myPlayList.Create(numElements); 
						myPlayList.PrintPlaylistInfo(); 
					}
					else if ( (operation.compare("play") == 0) && (tokens.size() > 1) )
					{
						int trackId = stoi(tokens[1]);
						if (!myPlayList.Play(trackId))
						{
							myPlayList.PrintPlaylistInfo();
						}
						else
						{
							cout << "Invalid Track" << endl; 
						}
					}
					else if ( (operation.compare("insert") == 0) && (tokens.size() > 2) )
					{
						int ordinal = stoi(tokens[1]);
						int trackId = stoi(tokens[2]);
						if (!myPlayList.Insert(ordinal, trackId))
						{
							myPlayList.PrintPlaylistInfo();
						}
						else
						{
							cout << "Invalid Track" << endl;
						}
					}
					else if ( ((operation.compare("delete") == 0) && tokens.size() > 1))
					{
						int trackId = stoi(tokens[1]);
						if (!myPlayList.Delete(trackId))
						{
							myPlayList.PrintPlaylistInfo();
						}
						else
						{
							cout << "Invalid Track" << endl;
						}
					}
					else if (operation.compare("shuffle") == 0)
					{
						if (!myPlayList.Shuffle())
						{
							myPlayList.PrintPlaylistInfo();
						}
						else
						{
							cout << "Nothing to shuffle. " << endl; 
						}
					}
					else
					{
						cout << "Invalid Operation specified - valid ops are create, play, insert, delete, shuffle. " << endl;
					}
				}


			}
		}
};

int main(int argc, char *argv[])
{
	/*
	TestDriver m_td; 

	m_td.TestShuffle(); 

	m_td.TestInsert(); 

	m_td.TestCreate(); 

	m_td.TestDelete();
	*/

	TestStdIn tstdIn; 


	return 0; 
}

