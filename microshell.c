#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

typedef struct files
{
	int infile;
	int outfile;
	int *fd;
	char **envp;
	int stuck;
}   t_files;

size_t  ft_strlen(char *s)
{
	char *c = s;

	if (!s)
		return (0);
	while (*s)
		s++;
	return (s - c);
}

void    child(char **start, char **end, t_files files)
{
	if (files.infile != 0)
		dup2(files.infile, STDIN_FILENO);
	if (files.outfile != 1)
		dup2(files.outfile, STDOUT_FILENO);
	if (files.fd[0] != -1)
	{
		close(files.fd[0]);
		close(files.fd[1]);
	}
	if (files.stuck != -1)
		close (files.stuck);
	start[end - start] = NULL;
	if (execve(start[0], start, files.envp) == -1)
	{
		write(2, "error: cannot execute ", ft_strlen("error: cannot execute "));
		write(2, start[0], ft_strlen(start[0]));
		write (2, "\n", 1);
		exit(1);
	}
}

void    cd(char **start, char **end)
{
	if (end - start != 1)
		write(2, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments\n"));
	else if (chdir(start[0]))
	{
		write(2, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to "));
		write (2, start[0], ft_strlen(start[0]));
		write (2, "\n", 1);
	}
}

void    execute(char **start, char **end, t_files files)
{
	int pid;

	pid = fork();
	if (pid == -1)
	{
		write (2, "error: fatal\n", ft_strlen("error: fatal\n"));
		exit(1);
	}
	if (!pid)
		child(start, end, files);
	if (pid)
		if (!files.infile && files.outfile == 1)
			waitpid(pid, NULL, 0);
}

void    command_line(char **start, char **end, t_files files)
{
	int i = 0;
	int fd[2];
	char **tmp = start;

	if (start == end)
		return;
	if (!strncmp(start[0], "cd", 3))
		return (cd(start + 1, end));
	while (start != end)
	{
		if (start[i][0] == '|')
		{
			pipe(fd);
			execute(tmp, start, (t_files){files.infile, fd[1], fd, files.envp, -1});
			close(fd[1]);
			if (files.stuck != -1)
				close(files.stuck);
			command_line(start + 1, end, (t_files){fd[0], files.outfile, fd, files.envp, fd[0]});
			close(fd[0]);
			return ;
		}
		start++;
	}
	execute(tmp, end, files);
	if (files.infile)
	{
		close(files.infile);
		while (wait(NULL) != -1)
			;
	}
}

void    split_lines(int ac, char **av, t_files files)
{
	int j = 0;

	while (j < ac)
	{
		if (av[j][0] == ';')
		{
			command_line(av, av + j, files);
			split_lines(ac - j - 1, av + j + 1, files);
			return ;
		}
		j++;
	}
	command_line(av, av + ac, files);
}

int main(int ac, char **av, char **envp)
{
	t_files files;
	int fd[2];
	if (ac > 1)
	{
		fd[0] = -1;
		fd[1] = -1;
		split_lines(ac - 1, av + 1, (t_files){0, 1, fd, envp, -1});
	}
}
