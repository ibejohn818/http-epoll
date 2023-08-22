from locust import FastHttpUser, HttpUser, task, between
import random

domains_frag = "ssl-{}.domain.{}"
domain_suffix = [
    "com",
    "net",
    "org",
    "dev"
]

MAX_DOMAINS = 100


class Basic(HttpUser):
    wait_time = between(0.5, 1.5)

    def domain(self):
        # assemble domain
        # suffix_key = random.randint(0, (len(domain_suffix) - 1))
        # suffix = domain_suffix[suffix_key]
        # site_key = random.randint(1, MAX_DOMAINS)
        # domain = domains_frag.format(site_key, suffix)

        # domain = "10.0.3.120:8080"
        domain = "0.0.0.0:8080"
        return domain
       

    @task
    def cpu(self):
        domain = self.domain()
        self.client.get("http://{}".format(domain), verify=False)
