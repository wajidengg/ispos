from setuptools import setup, find_packages

setup(
    name='ispos',
    version='1.0.0',
    packages=find_packages(),
    install_requires=[
        'opencv-python-headless',
        'numpy',
        'requests',
        'pillow',
        'pyserial',
    ],
    entry_points={
        'console_scripts': [
            'ispos=ispos.main:main',
        ],
    },
)