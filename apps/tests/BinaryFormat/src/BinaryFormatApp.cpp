#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BinaryFormatApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

struct ParticleData
{
    float Pos[3];
    float Vel[3];
    float Mass;
    int Type;
    
    float Rho, U, Temp, Ne;
};

struct io_header_1
{
    int npart[6];
    double mass[6];
    double time;
    double redshift;
    int flag_sfr;
    int flag_feedback;
    int npartTotal[6];
    int flag_cooling;
    int num_files;
    double BoxSize;
    double Omega0;
    double OmegaLambda;
    double HubbleParam;
    char fill[256 - 6 * 4 - 6 * 8 - 2 * 8 - 2 * 4 - 6 * 4 - 2 * 4 - 4 * 8];	/* fills to 256 Bytes */
} header1;



int NumPart, Ngas;

struct particle_data
{
    float Pos[3];
    float Vel[3];
    float Mass;
    int Type;
    
    float Rho, U, Temp, Ne;
} *P;

int *Id;

double Time, Redshift;



/* this routine allocates the memory for the
 * particle data.
 */
int allocate_memory(void)
{
    printf("allocating memory...\n");
    
    if(!(P = (particle_data*) malloc(NumPart * sizeof(struct particle_data))))
    {
        fprintf(stderr, "failed to allocate memory.\n");
        exit(0);
    }
    
    P--;				/* start with offset 1 */
    
    
    if(!(Id = (int*) malloc(NumPart * sizeof(int))))
    {
        fprintf(stderr, "failed to allocate memory.\n");
        exit(0);
    }
    
    Id--;				/* start with offset 1 */
    
    printf("allocating memory...done\n");
}

int load_snapshot( const char *fname, int files)
{
    FILE *fd;
    char buf[200];
    int i, j, k, dummy, ntot_withmasses;
    int t, n, off, pc, pc_new, pc_sph;
    
#define SKIP fread(&dummy, sizeof(dummy), 1, fd);
    
    for(i = 0, pc = 1; i < files; i++, pc = pc_new)
    {
        if(files > 1)
            sprintf(buf, "%s.%d", fname, i);
        else
            sprintf(buf, "%s", fname);
        
        if(!(fd = fopen(buf, "r")))
        {
            printf("can't open file `%s`\n", buf);
            exit(0);
        }
        
        printf("reading `%s' ...\n", buf);
        fflush(stdout);
        
        fread(&dummy, sizeof(dummy), 1, fd);
        fread(&header1, sizeof(header1), 1, fd);
        fread(&dummy, sizeof(dummy), 1, fd);
        
        if(files == 1)
        {
            for(k = 0, NumPart = 0, ntot_withmasses = 0; k < 6; k++)
                NumPart += header1.npart[k];
            Ngas = header1.npart[0];
        }
        else
        {
            for(k = 0, NumPart = 0, ntot_withmasses = 0; k < 6; k++)
                NumPart += header1.npartTotal[k];
            Ngas = header1.npartTotal[0];
        }
        
        for(k = 0, ntot_withmasses = 0; k < 6; k++)
        {
            if(header1.mass[k] == 0)
                ntot_withmasses += header1.npart[k];
        }
        
        if(i == 0)
            allocate_memory();
        
        SKIP;
        for(k = 0, pc_new = pc; k < 6; k++)
        {
            for(n = 0; n < header1.npart[k]; n++)
            {
                fread(&P[pc_new].Pos[0], sizeof(float), 3, fd);
                pc_new++;
            }
        }
        SKIP;
        
        SKIP;
        for(k = 0, pc_new = pc; k < 6; k++)
        {
            for(n = 0; n < header1.npart[k]; n++)
            {
                fread(&P[pc_new].Vel[0], sizeof(float), 3, fd);
                pc_new++;
            }
        }
        SKIP;
        
        
        SKIP;
        for(k = 0, pc_new = pc; k < 6; k++)
        {
            for(n = 0; n < header1.npart[k]; n++)
            {
                fread(&Id[pc_new], sizeof(int), 1, fd);
                pc_new++;
            }
        }
        SKIP;
        
        
        if(ntot_withmasses > 0)
            SKIP;
        for(k = 0, pc_new = pc; k < 6; k++)
        {
            for(n = 0; n < header1.npart[k]; n++)
            {
                P[pc_new].Type = k;
                
                if(header1.mass[k] == 0)
                    fread(&P[pc_new].Mass, sizeof(float), 1, fd);
                else
                    P[pc_new].Mass = header1.mass[k];
                pc_new++;
            }
        }
        if(ntot_withmasses > 0)
            SKIP;
        
        
        if(header1.npart[0] > 0)
        {
            SKIP;
            for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
            {
                fread(&P[pc_sph].U, sizeof(float), 1, fd);
                pc_sph++;
            }
            SKIP;
            
            SKIP;
            for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
            {
                fread(&P[pc_sph].Rho, sizeof(float), 1, fd);
                pc_sph++;
            }
            SKIP;
            
            if(header1.flag_cooling)
            {
                SKIP;
                for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
                {
                    fread(&P[pc_sph].Ne, sizeof(float), 1, fd);
                    pc_sph++;
                }
                SKIP;
            }
            else
                for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
                {
                    P[pc_sph].Ne = 1.0;
                    pc_sph++;
                }
        }
        
        fclose(fd);
    }
    
    
    Time = header1.time;
    Redshift = header1.time;
}


void BinaryFormatApp::setup()
{
    
    // get the absolute path to the data
    fs::path dataDirectory  = ( getAppPath().parent_path().parent_path().parent_path().parent_path().parent_path().parent_path() / "common/data" );
    fs::path p              = getOpenFilePath( dataDirectory );
    
    // quit if the file doesn't exists
    if( p.empty() || !fs::exists( p ) )
        quit();
    
    // open the file and parse it line by line
    ifstream file( p.c_str(), std::ios_base::binary );
    string line;
    
    
    load_snapshot( p.c_str(), 1 );
    
    cout << NumPart << " Particles " << endl;
    
    for( int i = 0; i < NumPart; i++ ){
        cout << "Position: " << P[i].Pos[0] << " " << P[i].Pos[1] << " " << P[i].Pos[2] << endl <<
        "Velocity: " << P[i].Vel[0] << " " << P[i].Vel[1] << " " << P[i].Vel[2] << " " << endl <<
        "Mass: " << P[i].Mass << " Type: " <<  P[i].Type << " Rho: " << P[i].Rho << endl <<
        "Ne: " << P[i].Ne << " Temp: " <<  P[i].Temp << " U: " << P[i].U << endl<< endl;
    }
    /*uint32_t index = 0;
    while( std::getline( file, line ) ){
        ParticleData data = *reinterpret_cast<const ParticleData*>( line.c_str() );
        cout << "Position: " << data.Pos[0] << " " << data.Pos[1] << " " << data.Pos[2] << endl <<
        "Velocity: " << data.Vel[0] << " " << data.Vel[1] << " " << data.Vel[2] << " " << endl <<
        "Mass: " << data.Mass << " Type: " <<  data.Type << " Rho: " << data.Rho << endl;;
    }*/
}

void BinaryFormatApp::mouseDown( MouseEvent event )
{
}

void BinaryFormatApp::update()
{
}

void BinaryFormatApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( BinaryFormatApp, RendererGl )
