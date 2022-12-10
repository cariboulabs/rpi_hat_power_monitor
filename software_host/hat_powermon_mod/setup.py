# -*- coding: utf-8 -*-

from setuptools import setup, find_packages

with open('README.rst') as f:
    readme = f.read()

with open('LICENSE') as f:
    license = f.read()

setup(
    name='hat_powermon',
    version='1.0.0',
    description='RPI HAT power monitor board driver',
    long_description=readme,
    author='David Michaeli / CaribouLabs LTD',
    author_email='cariboulabs.co@gmail.com',
    url='https://github.com/cariboulabs/rpi_hat_power_monitor.git',
    license=license,
    packages=find_packages(exclude=('tests'))
)