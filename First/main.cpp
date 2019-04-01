#define _CRT_SECURE_NO_WARNINGS
#include "GLUT.h"
#include <math.h>

/*

- For some reason - rand() is not giving randomize number - always the same number,
because of that i did the training without getting randomize flower photo.

- For the validation i check the last flower of the tulips .

*/

#include <stdio.h>
#include <time.h>
#include <Windows.h>

const int W = 600; // window width
const int H = 600; // window height

const int epochs = 10000;

const int TMPSZ = 512;
const int SCRSZ = 256;
const int INPUT_SZ = SCRSZ + 1;
const int HIDDEN_SZ = INPUT_SZ / 2 + 1;
const int OUTPUT_SZ = 3;

unsigned char picture[TMPSZ][TMPSZ][3]; // for R,G,B
unsigned char screen[SCRSZ][SCRSZ][3]; // for R,G,B
unsigned char squares[SCRSZ][SCRSZ][3]; // for R,G,B

double input[INPUT_SZ];
double hidden[HIDDEN_SZ];
double output[OUTPUT_SZ];
double i2h[INPUT_SZ][HIDDEN_SZ];
double h2o[HIDDEN_SZ][OUTPUT_SZ];
double error[OUTPUT_SZ];
double delta_output[OUTPUT_SZ];
double delta_hidden[HIDDEN_SZ];
int network_digit = -1, tutor_digit = -1;
double learning_rate = 0.1;

unsigned char* bmp;

char* flowers[17] = { "c1.bmp", "c2.bmp", "c3.bmp", "c4.bmp", "c5.bmp", "c6.bmp",
"r1.bmp", "r2.bmp" , "r3.bmp" , "r4.bmp" , "r5.bmp" , "r6.bmp" ,"t1.bmp", "t2.bmp" , "t3.bmp" , "t4.bmp" , "t5.bmp" };

void Clean();

void LoadBitmap(char * filename)
{
	int sz;
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	FILE* pf = fopen(filename, "rb"); // read binary file
	if (pf == NULL)
	{
		printf("Failed opening the photo.!\n");
		return;
	}
	fread(&bf, sizeof(bf), 1, pf);
	fread(&bi, sizeof(bi), 1, pf);
	sz = bi.biHeight * bi.biWidth * 3;

	bmp = (unsigned char*)malloc(sz);

	fread(bmp, 1, sz, pf);

	int k;
	int i, j;
	sz = TMPSZ*TMPSZ * 3;
	for (k = 0, j = 0, i = 0; k < sz; k += 3)
	{
		picture[i][j][2] = bmp[k]; //blue
		picture[i][j][1] = bmp[k + 1]; // green
		picture[i][j][0] = bmp[k + 2]; // red
		j++;
		if (j == TMPSZ) // fill next line
		{
			j = 0;
			i++;
		}
	}

	// copy picture to screen
	for (i = 0; i < SCRSZ; i++)
		for (j = 0; j < SCRSZ; j++)
		{
			screen[i][j][0] = (picture[i * 2][j * 2][0] + picture[i * 2][j * 2 + 1][0] +
				picture[i * 2 + 1][j * 2][0] + picture[i * 2 + 1][j * 2 + 1][0]) / 4;
			screen[i][j][1] = (picture[i * 2][j * 2][1] + picture[i * 2][j * 2 + 1][1] +
				picture[i * 2 + 1][j * 2][1] + picture[i * 2 + 1][j * 2 + 1][1]) / 4;
			screen[i][j][2] = (picture[i * 2][j * 2][2] + picture[i * 2][j * 2 + 1][2] +
				picture[i * 2 + 1][j * 2][2] + picture[i * 2 + 1][j * 2 + 1][2]) / 4;

		}
	free(bmp);
	fclose(pf);

}

void randomWeights()
{
	srand(time(0));
	// set random weights
	for (int i = 0; i < INPUT_SZ; i++)
		for (int j = 0; j < HIDDEN_SZ; j++)
			i2h[i][j] = ((rand() % 1000) - 500) / 1000.0;

	// set random weights
	for (int i = 0; i < HIDDEN_SZ; i++)
		for (int j = 0; j < OUTPUT_SZ; j++)
			h2o[i][j] = ((rand() % 1000) - 500) / 1000.0;
}

void init()
{
	int k, sz = TMPSZ*TMPSZ * 3;
	int i, j;

	Clean();

	LoadBitmap("t6.bmp");

	randomWeights();

	glClearColor(0.3, 0.3, 0.3, 0);

	glOrtho(-1, 1, -1, 1, -1, 1);
}

void DrawSquares()
{
	double top = 0.95;
	double left = 0.5;
	double right = 0.6;
	double bottom = 0.85;

	for (int i = 0; i < OUTPUT_SZ; i++)
	{
		/*if (tutor_digit == i)
			glColor3d(0, 0.7, 0);
		else if (network_digit == i)
			glColor3d(0.7, 0.6, 0.2);*/
		if (network_digit == i)
			glColor3d(0.7, 0.6, 0.2);

		else glColor3d(0.5, 0.5, 0.5);

		glBegin(GL_POLYGON);
		glVertex2d(left, top);
		glVertex2d(right, top);
		glVertex2d(right, bottom);
		glVertex2d(left, bottom);
		glEnd();

		glColor3d(1, 1, 1);

		glRasterPos2d(left + 0.02, bottom + 0.02);
		if (i == 0)
		{
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 'c');
		}
		else if (i == 1)
		{
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 'r');
		}
		else if (i == 2)
		{
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 't');
		}

		top -= 0.2;
		bottom -= 0.2;

	}


}

void HPF()
{
	int i, j;
	for (i = 1; i < SCRSZ - 1; i++)
		for (j = 1; j < SCRSZ - 1; j++)
			squares[i][j][0] = squares[i][j][1] = squares[i][j][2] =
			(int)fabs(4 * screen[i][j][0] - screen[i - 1][j][0] -
				screen[i + 1][j][0] - screen[i][j - 1][0] - screen[i][j + 1][0]);
}

void Digitize()
{
	int i, j, ii, jj, k, l;

	for (i = 0; i < SCRSZ; i++)
		for (j = 0; j < SCRSZ; j++)
		{
			if (screen[i][j][0] == 0)
			{
				ii = (i / 10) * 10; // starting point of square
				jj = (j / 10) * 10;
				for (k = ii; k < ii + 10; k++)
					for (l = jj; l < jj + 10; l++)
					{
						squares[k][l][0] = squares[k][l][1] = squares[k][l][2] = 0;
					}
			}

		}
	tutor_digit = -1;
}

int MaxOutput()
{
	int i, max = 0;
	for (i = 1; i < OUTPUT_SZ; i++)
		if (output[i] > output[max])
			max = i;
	return max;
}

void Clean()
{
	int i, j;
	for (i = 0; i < SCRSZ; i++)
		for (j = 0; j < SCRSZ; j++)
		{
			screen[i][j][0] = 255;
			screen[i][j][1] = 255;
			screen[i][j][2] = 255;
		}
	for (i = 0; i < SCRSZ; i++)
		for (j = 0; j < SCRSZ; j++)
		{
			squares[i][j][0] = 255;
			squares[i][j][1] = 255;
			squares[i][j][2] = 255;
		}

	network_digit = -1;

}

void FeedForward()
{
	int i, j;
	// 1. setup input layer
	for (i = 5; i < SCRSZ; i += 10)
		for (j = 5; j < SCRSZ; j += 10)
		{
			if (squares[i][j][0] == 0) // 
			{
				input[(i / 10) * 10 + (j / 10)] = 1;
				//printf("%c", 1);
			}
			else
			{
				input[(i / 10) * 10 + (j / 10)] = 0;
				//printf("%c", 2);
			}
			//if ((j / 10 + 1) % 10 == 0)
				//printf("\n");
		}
	input[INPUT_SZ - 1] = 1; // bias for input layer

							 // 2. getting Hidden layer
	for (i = 0; i < HIDDEN_SZ; i++)
		hidden[i] = 0;
	for (i = 0; i < INPUT_SZ; i++)
		for (j = 0; j < HIDDEN_SZ; j++)
		{
			hidden[j] += input[i] * i2h[i][j];
		}
	// add sigmoid
	for (i = 0; i < HIDDEN_SZ; i++)
		hidden[i] = 1 / (1 + exp(hidden[i]));

	// set bias for hidden layer
	hidden[HIDDEN_SZ - 1] = 1;
	// 3. getting output layer
	for (i = 0; i < OUTPUT_SZ; i++)
		output[i] = 0;
	for (i = 0; i < HIDDEN_SZ; i++)
		for (j = 0; j < OUTPUT_SZ; j++)
		{
			output[j] += hidden[i] * h2o[i][j];
		}
	// add sigmoid
	for (i = 0; i < OUTPUT_SZ; i++)
		output[i] = 1 / (1 + exp(output[i]));

	// show it
	//printf("\nOUTPUT\n");
	//for (i = 0; i < OUTPUT_SZ; i++)
		//printf("%.3lf ", output[i]);

	//printf("\n");

	network_digit = MaxOutput();

}

void Backpropagation()
{
	int i, j, k;
	// 1. Compute error E = (t(i)-y(i))
	for (i = 0; i < OUTPUT_SZ; i++)
	{
		if (i == tutor_digit)
			error[i] = (1 - output[i]);
		else
			error[i] = -output[i];
	}
	// 2. compute delta of output layer
	for (i = 0; i < OUTPUT_SZ; i++)
	{
		delta_output[i] = output[i] * (1 - output[i])*error[i];
	}
	// 3.  compute delta of hidden layer
	for (j = 0; j < HIDDEN_SZ; j++)
	{
		double tmp = 0;
		for (k = 0; k < OUTPUT_SZ; k++)
			tmp += delta_output[k] * h2o[j][k];

		delta_hidden[j] = hidden[j] * (1 - hidden[j])*tmp;
	}
	// 4. update weights in h2o
	for (i = 0; i < HIDDEN_SZ; i++)
		for (j = 0; j < OUTPUT_SZ; j++)
			h2o[i][j] -= learning_rate*hidden[i] * delta_output[j];
	// 5. update weights in i2h
	for (i = 0; i < INPUT_SZ; i++)
		for (j = 0; j < HIDDEN_SZ; j++)
			i2h[i][j] -= learning_rate*input[i] * delta_hidden[j];


	Clean();

}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	DrawSquares();
	// show screen
	glRasterPos2d(-0.95, 0);
	glDrawPixels(SCRSZ, SCRSZ, GL_RGB,
		GL_UNSIGNED_BYTE, screen);
	// show squares
	glRasterPos2d(-0.95, -0.95);
	glDrawPixels(SCRSZ, SCRSZ, GL_RGB,
		GL_UNSIGNED_BYTE, squares);



	glutSwapBuffers();// show what was drawn in "frame buffer"
}

void idle()
{
	glutPostRedisplay();// calls indirectly to display
}

void drag(int x, int y)
{
	int i, j;
	if (x > W / 4 && x<W / 4 + SCRSZ &&
		H - y>H / 2 && H - y < H / 2 + SCRSZ)
	{
		j = x - W / 4; // compute i,j in matrix
		i = H - y - H / 2;
		screen[i][j][0] = screen[i][j][1] = screen[i][j][2] = 0;
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (x > W / 4 && x<W / 4 + SCRSZ
			&& H - y >  H / 4 && H - y < H / 4 + SCRSZ) // click in bottom screen
		{
			//Digitize();
			HPF();
			FeedForward();
		}
		if (x > 3 * W / 4 && x < 3 * W / 4 + W / 20)
		{
			//tutor_digit = y / (H / 10);
			// start Backpropagation
			//Backpropagation();
		}
	}
}

void training()
{
	randomWeights();
	for (int i = 0; i < epochs; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			printf("EPOCH number : %d\n", i + 1);
			switch (flowers[j][0]) {
			case 'c':
				tutor_digit = 0;
				break;
			case 'r':
				tutor_digit = 1;
				break;
			case 't':
				tutor_digit = 2;
				break;
			}
			LoadBitmap(flowers[j]);
			//Digitize();
			HPF();
			FeedForward();
			Backpropagation();
		}
	}
}

void main(int argc, char* argv[])
{
	training();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("Digits Example ");

	glutDisplayFunc(display); // refresh function
	glutIdleFunc(idle); // idle: when nothing happens
	glutMotionFunc(drag); // 
	glutMouseFunc(mouse);
	init();

	glutMainLoop();
}