#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/server.h>

static const char kRSAPrivateKeyPEM[] =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEowIBAAKCAQEArAOREUWlBXJAKZ5hABYyxnRayDZP1bJeLbPVK+npxemrhHyZ\n"
    "gjdbY3ADot+JRyWjvll2w2GI+3blt0j+x/ZWwjMKu/QYcycYp5HL01goxOxuusZb\n"
    "i+KiHRGB6z0EMdXM7U82U7lA/j//HyZppyDjUDniWabXQJge8ksGXGTiFeAJ/687\n"
    "uV+JJcjGPxAGFQxzyjitf/FrL9S0WGKZbyqeGDzyeBZ1NLIuaiOORyLGSW4duHLD\n"
    "N78EmsJnwqg2gJQmRSaD4BNZMjtbfiFcSL9Uw4XQFTsWugUDEY1AU4c5g11nhzHz\n"
    "Bi9qMOt5DzrZQpD4j0gA2LOHpHhoOdg1ZuHrGQIDAQABAoIBAFJTaqy/jllq8vZ4\n"
    "TKiD900wBvrns5HtSlHJTe80hqQoT+Sa1cWSxPR0eekL32Hjy9igbMzZ83uWzh7I\n"
    "mtgNODy9vRdznfgO8CfTCaBfAzQsjFpr8QikMT6EUI/LpiRL1UaGsNOlSEvnSS0Z\n"
    "b1uDzAdrjL+nsEHEDJud+K9jwSkCRifVMy7fLfaum+YKpdeEz7K2Mgm5pJ/Vg+9s\n"
    "vI2V1q7HAOI4eUVTgJNHXy5ediRJlajQHf/lNUzHKqn7iH+JRl01gt62X8roG62b\n"
    "TbFylbheqMm9awuSF2ucOcx+guuwhkPir8BEMb08j3hiK+TfwPdY0F6QH4OhiKK7\n"
    "MTqTVgECgYEA0vmmu5GOBtwRmq6gVNCHhdLDQWaxAZqQRmRbzxVhFpbv0GjbQEF7\n"
    "tttq3fjDrzDf6CE9RtZWw2BUSXVq+IXB/bXb1kgWU2xWywm+OFDk9OXQs8ui+MY7\n"
    "FiP3yuq3YJob2g5CCsVQWl2CHvWGmTLhE1ODll39t7Y1uwdcDobJN+ECgYEA0LlR\n"
    "hfMjydWmwqooU9TDjXNBmwufyYlNFTH351amYgFUDpNf35SMCP4hDosUw/zCTDpc\n"
    "+1w04BJJfkH1SNvXSOilpdaYRTYuryDvGmWC66K2KX1nLErhlhs17CwzV997nYgD\n"
    "H3OOU4HfqIKmdGbjvWlkmY+mLHyG10bbpOTbujkCgYAc68xHejSWDCT9p2KjPdLW\n"
    "LYZGuOUa6y1L+QX85Vlh118Ymsczj8Z90qZbt3Zb1b9b+vKDe255agMj7syzNOLa\n"
    "/MseHNOyq+9Z9gP1hGFekQKDIy88GzCOYG/fiT2KKJYY1kuHXnUdbiQgSlghODBS\n"
    "jehD/K6DOJ80/FVKSH/dAQKBgQDJ+apTzpZhJ2f5k6L2jDq3VEK2ACedZEm9Kt9T\n"
    "c1wKFnL6r83kkuB3i0L9ycRMavixvwBfFDjuY4POs5Dh8ip/mPFCa0hqISZHvbzi\n"
    "dDyePJO9zmXaTJPDJ42kfpkofVAnfohXFQEy+cguTk848J+MmMIKfyE0h0QMabr9\n"
    "86BUsQKBgEVgoi4RXwmtGovtMew01ORPV9MOX3v+VnsCgD4/56URKOAngiS70xEP\n"
    "ONwNbTCWuuv43HGzJoVFiAMGnQP1BAJ7gkHkjSegOGKkiw12EPUWhFcMg+GkgPhc\n"
    "pOqNt/VMBPjJ/ysHJqmLfQK9A35JV6Cmdphe+OIl28bcKhAOz8Dw\n"
    "-----END RSA PRIVATE KEY-----\n";


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    
	//create a pair of connected sockets
	int socket_fds[2];
    int res = socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fds);
    assert(res >= 0);
	
	// send input to connected socket
    ssize_t send_res = send(socket_fds[1], data, size, 0);
    assert(send_res == size);

	// shoutdown socket after sending data
    res = shutdown(socket_fds[1], SHUT_WR);
    assert(res == 0);

	// Open temporal file which contains RSA private key 
    int fd = open("/tmp/libssh_fuzzer_private_key", O_WRONLY | O_CREAT, S_IRWXU);
    assert(fd >= 0);
    
	ssize_t write_res = write(fd, kRSAPrivateKeyPEM, strlen(kRSAPrivateKeyPEM));
    assert(write_res == strlen(kRSAPrivateKeyPEM));
    close(fd);
	
	//Create a new SSH server bind and session
    ssh_bind sshbind = ssh_bind_new(); // A newly allocated ssh_bind session pointer
    ssh_session session = ssh_new(); // A new ssh_seesion pointer

    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, "/tmp/libssh_fuzzer_private_key");
    
	//Accepting an incoming SSH connection on the given file descriptor and initialize the session
    res = ssh_bind_accept_fd(sshbind, session, socket_fds[0]);
    assert(res == SSH_OK);

	//Retrieve message through SSH
    if (ssh_handle_key_exchange(session) == SSH_OK) {
        while (true) {
            ssh_message message = ssh_message_get(session);
            if (!message) {
                break;
            }
            ssh_message_free(message);
        }
    }

    close(socket_fds[0]);
    close(socket_fds[1]);

    ssh_disconnect(session);
    ssh_free(session);
    ssh_bind_free(sshbind);

    return 0;
}
