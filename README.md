# Sporkbomb VCV Rack plugin

## Entropy Pool

![Entropy Pool](./res/EntropyPool.png)

A sequencer that slices from a pool of random data

### Github integration

The context menu includes an "Integrations..." item, which lets you use a Github token to use a
user's contribution history (the green boxes on their profile page) as the data source.

To load private activity, use a classic token with the `repo` and `read:user` permissions.

The token is *not* saved with patches, but the loaded activity is, so you don't need to re-enter your token every time.

# Development

## Setup

1. `git submodule init`
2. `make && make install`
